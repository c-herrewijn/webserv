/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 17:56:25 by fra           #+#    #+#                 */
/*   Updated: 2023/12/08 04:00:01 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"
#include "CGI.hpp"

WebServer::WebServer( std::vector<ConfigServer> const& servers ) : _servers(servers)
{
	bool defServerFound = false;

	if (servers.empty() == true)
		throw(ServerException({"no Servers provided for configuration"}));
	for (auto const& server : this->_servers)
	{
		for (auto const& listAddress : server.getListens())
		{
			if (listAddress.getDef() == true)
			{
				this->_defaultServer = server;
				defServerFound = true;
			}
			auto isPresent = std::find(this->_listenAddress.begin(),this->_listenAddress.end(), listAddress);
			if ( isPresent == this->_listenAddress.end() )
				this->_listenAddress.push_back(listAddress);
		}
	}
	if (defServerFound == false)
		this->_defaultServer = servers[0];
}

WebServer::~WebServer ( void ) noexcept
{
	for (auto &item : this->_requests)
		delete item.second;
	for (auto &item : this->_responses)
		delete item.second;
	for (auto &item : this->_cgi)
		delete item.second;
	for (auto &item : this->_pollitems)
	{
		if ((item.second.pollType == LISTENER) or
			(item.second.pollType == CLIENT_CONNECTION))
			shutdown(item.first, SHUT_RDWR);
		close(item.first);
	}
}

void			WebServer::startListen( void )
{
	for (auto const& listAddress : this->_listenAddress)
	{
		try {
			this->_listenTo(listAddress.getIpString(), listAddress.getPortString());
		}
		catch(const ServerException& e) {
			std::cout << e.what() << '\n';
		}
	}
	if (this->_pollfds.empty() == true)
		throw(ServerException({"no available host:port in the configuration provided"}));
}

// NB: update for codes 20X
// NB: use relative root in Config File for multi-platform functionality
void			WebServer::loop( void )
{
	int				nConn=-1;
	struct pollfd 	pollfdItem;

	while (true)
	{
		nConn = poll(this->_pollfds.data(), this->_pollfds.size(), 0);
		if (nConn < 0)
		{
			if ((errno != EAGAIN) and (errno != EWOULDBLOCK))
				throw(ServerException({"poll failed"}));
			else
				continue;
		}
		else if (nConn == 0)
			continue;
		for(size_t i=0; i<this->_pollfds.size(); i++)
		{
			pollfdItem = this->_pollfds[i];
			try {
				if (pollfdItem.revents & POLLIN)
					_readData(pollfdItem.fd);
				else if (pollfdItem.revents & POLLOUT)
					_writeData(pollfdItem.fd);
				else if (pollfdItem.revents & (POLLHUP | POLLERR | POLLNVAL))	// client-end side was closed / error / socket not valid
					this->_emptyConns.push_back(pollfdItem.fd);
			}
			catch (const ServerException& e) {
				std::cerr << C_RED << e.what()  << C_RESET << '\n';
				this->_emptyConns.push_back(pollfdItem.fd);
			}
			catch (const EndConnectionException& e) {
				std::cout << C_GREEN << "CLOSED CONNECTION - " << pollfdItem.fd << C_RESET << std::endl;
				this->_emptyConns.push_back(pollfdItem.fd);
			}
			catch (const HTTPexception& e) {
				std::cerr << C_RED << e.what()  << C_RESET << '\n';
				redirectToErrorPage(pollfdItem.fd, e.getStatus());
			}
		_clearEmptyConns();
		}
	}
}

std::string		WebServer::_getAddress( const struct sockaddr_storage *addr ) const noexcept
{
	std::string ipAddress;
	if (addr->ss_family == AF_INET)
	{
		char ipv4[INET_ADDRSTRLEN];
		struct sockaddr_in *addr_v4 = (struct sockaddr_in*) addr;
		inet_ntop(addr_v4->sin_family, &(addr_v4->sin_addr), ipv4, sizeof(ipv4));
		ipAddress = std::string(ipv4) + std::string(":") + std::to_string(ntohs(addr_v4->sin_port));
	}
	else if (addr->ss_family == AF_INET6)
	{
		char ipv6[INET6_ADDRSTRLEN];
		struct sockaddr_in6 *addr_v6 = (struct sockaddr_in6*) addr;
		inet_ntop(addr_v6->sin6_family, &(addr_v6->sin6_addr), ipv6, sizeof(ipv6));
		ipAddress = std::string(ipv6) + std::string(":") + std::to_string(ntohs(addr_v6->sin6_port));
	}
	return (ipAddress);
}

ConfigServer const&	WebServer::_getHandler( std::string const& servName ) const noexcept
{
	std::string	tmpServName = servName;

	for (auto const& server : this->_servers)
	{
		for (auto name : server.getNames())
		{
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			std::transform(tmpServName.begin(), tmpServName.end(), tmpServName.begin(), ::tolower);
			if (tmpServName == name)
				return (server);
		}
	}
	return (_getDefaultHandler());
}

ConfigServer const&	WebServer::_getDefaultHandler( void ) const noexcept
{
	return (this->_defaultServer);
}

void	WebServer::_listenTo( std::string const& hostname, std::string const& port )
{
	struct addrinfo *tmp, *list, filter;
	struct sockaddr_storage	hostIP;
	int yes=1, listenSocket=-1;

	bzero(&filter, sizeof(struct addrinfo));
	filter.ai_flags = AI_PASSIVE;
	filter.ai_family = AF_UNSPEC;
	filter.ai_protocol = IPPROTO_TCP;
	if (getaddrinfo(hostname.c_str(), port.c_str(), &filter, &list) != 0)
		throw(ServerException({"failed to get addresses for", hostname, ":", port}));
	for (tmp=list; tmp!=nullptr; tmp=tmp->ai_next)
	{
		listenSocket = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
		if (listenSocket == -1)
			continue;
		fcntl(listenSocket, F_SETFL, O_NONBLOCK);
		if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0)
			std::cout << "failed to update socket, trying to bind anyway... \n";
		if (bind(listenSocket, tmp->ai_addr, tmp->ai_addrlen) == 0)
			break;
		close(listenSocket);
	}
	if (tmp == nullptr)
	{
		freeaddrinfo(list);
		std::cout << "no available IP host found for " << hostname << " port " << port << '\n';
		return ;
	}
	memmove(&hostIP, tmp->ai_addr, std::min(sizeof(struct sockaddr), sizeof(struct sockaddr_storage)));
	freeaddrinfo(list);
	if (listen(listenSocket, BACKLOG) != 0)
	{
		close(listenSocket);
		throw(ServerException({"failed listen on", this->_getAddress(&hostIP)}));
	}
	std::cout << "listening on: " << this->_getAddress(&hostIP) << "\n";
	this->_addConn(listenSocket, LISTENER, WAITING_FOR_CONNECTION);
}

void	WebServer::_readData( int readFd )	// POLLIN
{
	switch (this->_pollitems[readFd].pollState)
	{
		case WAITING_FOR_CONNECTION:
			std::cout << C_GREEN << "NEW_CONNECTION - " << readFd << C_RESET << std::endl;
			handleNewConnections(readFd);
			break;

		case READ_REQ_HEADER:
			std::cout << C_GREEN << "READ_REQ_HEADER - " << readFd << C_RESET << std::endl;
			readRequestHeaders(readFd);
			break;

		case FORWARD_REQ_BODY_TO_CGI:
			std::cout << C_GREEN << "POLLIN - FORWARD_REQ_BODY_TO_CGI - " << readFd << C_RESET << std::endl;
			readRequestBody(readFd);
			break;

		case READ_CGI_RESPONSE:
			std::cout << C_GREEN << "READ_CGI_RESPONSE " << readFd << C_RESET << std::endl;
			readCGIResponses(readFd);
			break;

		case READ_STATIC_FILE:
			std::cout << C_GREEN << "READ_STATIC_FILE - " << readFd << C_RESET << std::endl;
			readStaticFiles(readFd);
			break;

		default:
			break;		// NB: or throw exception?
	}
}

void	WebServer::_writeData( int writeFd )	// POLLOUT
{
	switch (this->_pollitems[writeFd].pollState)
	{
		case FORWARD_REQ_BODY_TO_CGI:
			std::cout << C_GREEN << "POLLOUT - FORWARD_REQ_BODY_TO_CGI - " << writeFd << C_RESET << std::endl;
			writeToCGI(writeFd);
			break;

		case WRITE_TO_CLIENT:
			std::cout << C_GREEN << "WRITE_TO_CLIENT - " << writeFd << C_RESET << std::endl;
			writeToClients(writeFd);
			break;

		default:
			break;		// NB: or throw exception?
	}
}

void	WebServer::_addConn( int newSocket , fdType typePollItem, fdState statePollItem )
{
	if (newSocket == -1)
		throw(ServerException({"invalid file descriptor"}));
	this->_pollfds.push_back({newSocket, POLLIN | POLLOUT, 0});
	t_PollItem newPollitem = {newSocket, typePollItem, statePollItem};
	this->_pollitems.insert(std::pair(newSocket, newPollitem));
}

void	WebServer::_dropConn(int toDrop) noexcept
{
	shutdown(toDrop, SHUT_RDWR);
	close(toDrop);
	for (auto curr=this->_pollfds.begin(); curr != this->_pollfds.end(); curr++)
	{
		if (curr->fd == toDrop)
		{
			this->_pollfds.erase(curr);
			break;
		}
	}
	if ((this->_pollitems[toDrop].pollType == CGI_DATA_PIPE) or
		(this->_pollitems[toDrop].pollType == CGI_RESPONSE_PIPE))
		{}						//NB: close pipes properly
	if (this->_requests[toDrop])
		delete this->_requests[toDrop];
	this->_requests.erase(toDrop);
	if (this->_responses[toDrop])
		delete this->_responses[toDrop];
	this->_responses.erase(toDrop);
	if (this->_cgi[toDrop])
		delete this->_cgi[toDrop];
	this->_cgi.erase(toDrop);
	this->_pollitems.erase(toDrop);
}

void	WebServer::_clearEmptyConns( void ) noexcept
{
	while (this->_emptyConns.empty() == false)
	{
		_dropConn(this->_emptyConns.back());
		this->_emptyConns.pop_back();
	}
}

int		WebServer::_getSocketFromFd( int fd )
{
	if (this->_pollitems[fd].pollType == CGI_DATA_PIPE)
	{
		for (auto& item : this->_cgi)
		{
			if (*item.second->getuploadPipe() == fd)
				return (item.second->getRequestSocket());
		}
	}
	else if (this->_pollitems[fd].pollType == CGI_RESPONSE_PIPE)
	{
		for (auto& item : this->_cgi)
		{
			if (*item.second->getResponsePipe() == fd)
				return (item.second->getRequestSocket());
		}
	}
	else if (this->_pollitems[fd].pollType == STATIC_FILE)
	{
		for (auto& item : this->_responses)
		{
			if (item.second->getHTMLfd() == fd)
				return (item.first);
		}
	}
	else if (fd != -1)
		return (fd);
	throw(ServerException({"invalid file descriptor or not found"}));	// entity not found, this should not happen
}

//NB: building absolut path?
// NB: fix after validation is ok
t_path	WebServer::_getHTMLerrorPage( int statusCode, HTTPrequest* request ) const
{
	(void) request;
	// try {
	// 	return (request->getErrorPages().at(statusCode));
	// }
	// catch(const std::out_of_range& e1)
	// {
	// 	try {
	// 		return (_getHandler(request->getHost()).getParams().getErrorPages().at(statusCode));
	// 	}
	// 	catch(const std::out_of_range& e2) {
	// 		try {
	// 			return (_getDefaultHandler().getParams().getErrorPages().at(statusCode));
	// 		}
	// 		catch(const std::out_of_range& e3) {
				for (auto const& dir_entry : std::filesystem::directory_iterator{HTML_ERROR_FOLDER})
				{
					if (dir_entry.path().stem() == std::to_string(statusCode))
						return (dir_entry.path());
				}
				throw(HTTPexception({"absolutely no HTML found for code:", std::to_string(statusCode)}, 500));
	// 		}
	// 	}
	// }
}

void	WebServer::handleNewConnections( int listenerFd )
{
	struct sockaddr_storage client;
	unsigned int 			sizeAddr = sizeof(client);
	int 					connFd = -1;

	connFd = accept(listenerFd, (struct sockaddr *) &client, &sizeAddr);
	if (connFd == -1)
		throw(ServerException({"connection with", this->_getAddress(&client), "failed"}));
	fcntl(connFd, F_SETFL, O_NONBLOCK);
	this->_addConn(connFd, CLIENT_CONNECTION, READ_REQ_HEADER);
	std::cout << "connected to " << this->_getAddress(&client) << '\n';
}

void	WebServer::readRequestHeaders( int clientSocket )
{
	HTTPrequest 	*request = nullptr;
	HTTPresponse	*response = nullptr;
	CGI 			*cgiPtr = nullptr;
	int 			HTMLfd = -1;
	fdState			nextState;

	request = new HTTPrequest(clientSocket);
	response = new HTTPresponse(clientSocket);
	this->_requests.insert(std::pair<int, HTTPrequest*>(clientSocket, request));
	this->_responses.insert(std::pair<int, HTTPresponse*>(clientSocket, response));
	request->parseMain();
	// std::cout << request->getRealPath() << "\n";
	if (!std::filesystem::exists(request->getRealPath()))	// NB: temporary until validation works
		throw(RequestException({"file not found"}, 404));
	// request->validateRequest(_getHandler(request->getHost()));
	if (request->isCGI()) {		// GET (CGI), POST and DELETE
		cgiPtr = new CGI(*request);
		this->_cgi.insert(std::pair<int, CGI*>(clientSocket, cgiPtr));
		this->_addConn(cgiPtr->getResponsePipe()[0], CGI_RESPONSE_PIPE, READ_CGI_RESPONSE);
		cgiPtr->run();
		if (request->hasBody()) {
			this->_addConn(cgiPtr->getuploadPipe()[1], CGI_DATA_PIPE, FORWARD_REQ_BODY_TO_CGI);
			nextState = FORWARD_REQ_BODY_TO_CGI;
		}
		else
			nextState = READ_CGI_RESPONSE;
	}
	else if (request->isAutoIndex())	//	GET (index)
	{
		response->readContentDirectory(request->getRealPath());
		nextState = WRITE_TO_CLIENT;
	}
	else		// GET (HTML)
	{
		HTMLfd = open(request->getRealPath().c_str(), O_RDONLY);
		response->setHTMLfd(HTMLfd);
		_addConn(HTMLfd, STATIC_FILE, READ_STATIC_FILE);
		nextState = READ_STATIC_FILE;
	}
	this->_pollitems[clientSocket].pollState = nextState;
}

void	WebServer::readStaticFiles( int staticFileFd )
{
	int 			socket = _getSocketFromFd(staticFileFd);
	HTTPresponse	*response = nullptr;

	response = this->_responses.at(socket);
	if (!response)	// that should not happen
		throw(ServerException({"response not found"}));
	response->readHTML();
	if (response->isDoneReadingHTML())
	{
		this->_emptyConns.push_back(staticFileFd);
		this->_pollitems[response->getSocket()].pollState = WRITE_TO_CLIENT;
	}
}

void	WebServer::readRequestBody( int clientSocket )
{
	HTTPrequest *request = this->_requests[clientSocket];
	if (request->getTmpBody() == "") {
		request->readPlainBody();
	}
}

// NB: it does a lot (like a lot) of calls for sending the body, if the exception
// is de-commented it falls into that, failing the upload
void	WebServer::writeToCGI( int cgiPipe )
{
	HTTPrequest *request = nullptr;
	for (auto& cgiMapItem : this->_cgi) {
		if (cgiMapItem.second->getuploadPipe()[1] == cgiPipe) {
			request = this->_requests[cgiMapItem.first];
			break;
		}
	}
	if (request != nullptr) {
		close(this->_cgi[request->getSocket()]->getuploadPipe()[0]); // close read end of cgi upload pipe
		std::string tmpBody = request->getTmpBody();
		if (tmpBody != "") {
			write(cgiPipe, tmpBody.data(), tmpBody.length());
			request->setTmpBody("");

			// drop from pollList after writing is done
			if (request->gotFullBody()) {
				close(cgiPipe); // close write end of cgi upload pipe
				this->_emptyConns.push_back(cgiPipe);
				this->_pollitems[request->getSocket()].pollState = READ_CGI_RESPONSE;
			}
		}
	}
	// else
	// 	throw(ServerException({"request not found"}));
}

void	WebServer::readCGIResponses( int cgiPipe )
{
	int 			socket = _getSocketFromFd(cgiPipe);
	CGI				*cgi = nullptr;
	HTTPresponse	*response = nullptr;

	cgi = this->_cgi.at(socket);
	ssize_t	readChars = -1;
	char 	buffer[DEF_BUF_SIZE];
	bzero(buffer, DEF_BUF_SIZE);
	readChars = read(cgiPipe, buffer, DEF_BUF_SIZE);
	if (readChars < 0)
		throw(ServerException({"unavailable socket"}));
	cgi->appendResponse(std::string(buffer, buffer + readChars));

	// TODO: support partial reads, i.e. in case of very big CGI response
	// a logic to understand when the whole CGI response is received is necessary
	// to know when to close the pipe connection
	if (true) // TODO keep pipe open for consequtive reads if needed
	{
		response = this->_responses[socket];
		response->parseFromCGI(cgi->getResponse());
		this->_emptyConns.push_back(cgiPipe);
		this->_pollitems[socket].pollState = WRITE_TO_CLIENT;
	}
}

void	WebServer::writeToClients( int clientSocket )
{
	HTTPrequest 	*request = this->_requests[clientSocket];
	HTTPresponse 	*response = this->_responses[clientSocket];

	if (!request or !response)	// that should not happen
		throw(ServerException({"request or response not found"}));
	if ((request->isCGI()) and (response->getStatusCode() < 400)) {
		CGI *cgi = this->_cgi[clientSocket];
		if (!cgi)	// that should not happen
			throw(ServerException({"cgi not found"}));
		response->parseFromCGI(cgi->getResponse());
		delete (cgi);
		this->_cgi.erase(clientSocket);
	}
	else
	{
		response->setServName(_getHandler(request->getHost()).getPrimaryName());
		response->parseFromStatic();
	}
	response->writeContent();
	if (response->isDoneWriting())
	{
		if (request->isEndConn())
			this->_emptyConns.push_back(clientSocket);
		else
			this->_pollitems[clientSocket].pollState = READ_REQ_HEADER;
		delete (response);
		this->_responses.erase(clientSocket);
		delete (request);
		this->_requests.erase(clientSocket);
	}
}

void	WebServer::redirectToErrorPage( int genericFd, int statusCode ) noexcept
{
	HTTPresponse	*response = nullptr;
	HTTPrequest		*request = nullptr;
	int				clientSocket=_getSocketFromFd(genericFd), HTMLfd=-1;
	t_path			HTMLerrPage;

	if ((this->_pollitems[genericFd].pollType == STATIC_FILE) or
		(this->_pollitems[genericFd].pollType == CGI_DATA_PIPE) or		// NB: pipes need to be closed differently see _dropConn()
		(this->_pollitems[genericFd].pollType == CGI_RESPONSE_PIPE))
		this->_emptyConns.push_back(genericFd);
	request = this->_requests.at(clientSocket);
	response = this->_responses.at(clientSocket);
	response->setStatusCode(statusCode);
	if (statusCode == 500)
	{
		this->_pollitems[clientSocket].pollState = WRITE_TO_CLIENT;
		return ;
	}
	try {
		HTMLerrPage = _getHTMLerrorPage(statusCode, request);
		HTMLfd = open(HTMLerrPage.c_str(), O_RDONLY);
		response->setHTMLfd(HTMLfd);
		_addConn(HTMLfd, STATIC_FILE, READ_STATIC_FILE);
		this->_pollitems[clientSocket].pollState = READ_STATIC_FILE;
	}
	catch(const WebServerException& e) {
		std::cerr << e.what() << '\n';
		response->setStatusCode(500);
		this->_pollitems[clientSocket].pollState = WRITE_TO_CLIENT;
	}
}
