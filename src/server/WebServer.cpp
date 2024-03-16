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
	bool 				defServerFound = false;
	std::vector<Listen>	listeners;

	if (servers.empty() == true)
		throw(ServerException({"no Servers provided for configuration"}));
	for (auto const& server : this->_servers)
	{
		for (auto const& address : server.getListens())
		{
			if (address.getDef() == true)
			{
				this->_defaultServer = server;
				defServerFound = true;
			}
			auto isPresent = std::find(listeners.begin(), listeners.end(), address);
			if ( isPresent == listeners.end() )
				listeners.push_back(address);
		}
	}
	if (defServerFound == false)
		this->_defaultServer = servers[0];
	for (auto const& listener : listeners)
	{
		try {
			this->_listenTo(listener.getIpString(), listener.getPortString());
		}
		catch(const ServerException& e) {
			std::cout << C_RED << e.what() << '\n' << C_RESET;
		}
	}
	if (this->_pollfds.empty() == true)
		throw(ServerException({"no available host:port in the configuration provided"}));
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
		if ((item.second->pollType == LISTENER) or
			(item.second->pollType == CLIENT_CONNECTION))
			shutdown(item.first, SHUT_RDWR);
		close(item.first);
		delete item.second;
	}
}

// NB: update for codes 20X
// NB: use relative root in Config File for multi-platform functionality
// NB: remove static variable from HTTPresponse.writeContent()
// NB: in file upload the Pipe upload is not dropped correctly
// NB: if an error occurs while CGI is running (fileupload) the cgi has to be stopped
void			WebServer::run( void )
{
	int				nConn = -1;
	struct pollfd 	pollfdItem;

	while (true)
	{
		nConn = poll(this->_pollfds.data(), this->_pollfds.size(), 0);
		if (nConn < 0)
		{
			if ((errno != EAGAIN) and (errno != EWOULDBLOCK))
				throw(ServerException({"poll failed"}));
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
				if (pollfdItem.revents & POLLOUT)
					_writeData(pollfdItem.fd);
				if (pollfdItem.revents & (POLLHUP | POLLERR | POLLNVAL))	// client-end side was closed / error / socket not valid
					this->_emptyConns.push_back(pollfdItem.fd);
			}
			catch (const ServerException& e) {
				std::cerr << C_RED << e.what() << C_RESET << '\n';
				this->_emptyConns.push_back(pollfdItem.fd);
			}
			catch (const EndConnectionException& e) {
				std::cout << C_GREEN << "CLOSED CONNECTION - " << pollfdItem.fd << C_RESET << std::endl;
				this->_emptyConns.push_back(pollfdItem.fd);
			}
			catch (const std::out_of_range& e) {
				std::cerr << C_RED << "entity not found"  << C_RESET << '\n';
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
			std::cout << C_RED << "failed to update socket, trying to bind anyway... \n" << C_RESET;
		if (bind(listenSocket, tmp->ai_addr, tmp->ai_addrlen) == 0)
			break;
		close(listenSocket);
	}
	if (tmp == nullptr)
	{
		freeaddrinfo(list);
		std::cout << C_RED << "no available IP host found for " << hostname << " port " << port << '\n' << C_RESET;
		return ;
	}
	memmove(&hostIP, tmp->ai_addr, std::min(sizeof(struct sockaddr), sizeof(struct sockaddr_storage)));
	freeaddrinfo(list);
	if (listen(listenSocket, BACKLOG) != 0)
	{
		close(listenSocket);
		throw(ServerException({"failed listen on", this->_getAddress(&hostIP)}));
	}
	std::cout << C_GREEN << "listening on: " << this->_getAddress(&hostIP) << "\n" << C_RESET;
	this->_addConn(listenSocket, LISTENER, WAITING_FOR_CONNECTION);
}

void	WebServer::_readData( int readFd )	// POLLIN
{
	switch (this->_pollitems[readFd]->pollState)
	{
		case WAITING_FOR_CONNECTION:
			std::cout << C_GREEN << "NEW_CONNECTION - " << readFd << C_RESET << std::endl;
			handleNewConnections(readFd);
			break;

		case READ_REQ_HEADER:
			std::cout << C_GREEN << "READ_REQ_HEADER - " << readFd << C_RESET << std::endl;
			readRequestHeaders(readFd);
			break;

		case READ_REQ_BODY:
			std::cout << C_GREEN << "READ_REQ_BODY - " << readFd << C_RESET << std::endl;
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
	switch (this->_pollitems[writeFd]->pollState)
	{
		case WRITE_TO_CGI:
			std::cout << C_GREEN << "WRITE_TO_CGI - " << writeFd << C_RESET << std::endl;
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
	t_PollItem *newPollitem = nullptr;

	if (newSocket == -1)
		throw(ServerException({"invalid file descriptor"}));
	this->_pollfds.push_back({newSocket, POLLIN | POLLOUT, 0});
	newPollitem = new PollItem;
	newPollitem->fd = newSocket;
	newPollitem->pollType = typePollItem;
	newPollitem->pollState = statePollItem;
	this->_pollitems[newSocket] = newPollitem;
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
	if ((this->_pollitems[toDrop]->pollType == CGI_REQUEST_PIPE) or
		(this->_pollitems[toDrop]->pollType == CGI_RESPONSE_PIPE))
		{}						//NB: close pipes properly
	delete this->_pollitems[toDrop];
	this->_pollitems.erase(toDrop);
	_dropStructs(toDrop);
}

void	WebServer::_dropStructs( int toDrop ) noexcept
{
	if (this->_requests.count(toDrop) > 0)
	{
		delete this->_requests[toDrop];
		this->_requests.erase(toDrop);
	}
	if (this->_responses.count(toDrop) > 0)
	{
		delete this->_responses[toDrop];
		this->_responses.erase(toDrop);
	}
	if (this->_cgi.count(toDrop) > 0)
	{
		delete this->_cgi[toDrop];
		this->_cgi.erase(toDrop);
	}
}

void	WebServer::_clearEmptyConns( void ) noexcept
{
	while (this->_emptyConns.empty() == false)
	{
		_dropConn(this->_emptyConns.back());
		this->_emptyConns.pop_back();
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
	if (addr->ss_family == AF_INET6)
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

int		WebServer::_getSocketFromFd( int fd )
{
	if (this->_pollitems[fd]->pollType == CGI_REQUEST_PIPE)
	{
		for (auto& item : this->_cgi)
		{
			if (*item.second->getuploadPipe() == fd)
				return (item.second->getRequestSocket());
		}
	}
	else if (this->_pollitems[fd]->pollType == CGI_RESPONSE_PIPE)
	{
		for (auto& item : this->_cgi)
		{
			if (*item.second->getResponsePipe() == fd)
				return (item.second->getRequestSocket());
		}
	}
	else if (this->_pollitems[fd]->pollType == STATIC_FILE)
	{
		for (auto& item : this->_responses)
		{
			if (item.second->getHTMLfd() == fd)
				return (item.first);
		}
	}
	else if (fd != -1)
		return (fd);
	throw(ServerException({"invalid file descriptor or not found:", std::to_string(fd)}));	// entity not found, this should not happen
}

// NB: building absolut path?
// NB: fix after validation is ok
t_path	WebServer::_getHTMLerrorPage( int statusCode, t_string_map const& localErrPages ) const
{
	(void) localErrPages;
	// try {
	// 	return (localErrPages.at(statusCode));
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
				throw(HTTPexception({"absolutely no HTML found for code", std::to_string(statusCode)}, 500));
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
	{
		std::cerr << C_RED  << "connection with: " << this->_getAddress(&client) << " failed\n" << C_RESET;
		return;
	}
	fcntl(connFd, F_SETFL, O_NONBLOCK);
	this->_addConn(connFd, CLIENT_CONNECTION, READ_REQ_HEADER);
	std::cout << C_GREEN << "connected to " << this->_getAddress(&client) << '\n' << C_RESET;
}

void	WebServer::readRequestHeaders( int clientSocket )
{
	HTTPrequest 	*request = nullptr;

	if (!this->_requests[clientSocket])
	{
		this->_requests[clientSocket] = new HTTPrequest(clientSocket);
		this->_responses[clientSocket] = new HTTPresponse(clientSocket);
	}
	request = this->_requests[clientSocket];
	request->parseHead();
	if (request->gotFullHead() == false)
		return ;
	request->validateRequest(_getHandler(request->getHost()));
	if (request->isCGI()) {		// GET (CGI), POST and DELETE
		_startCGI(clientSocket);
		if (request->hasBody())
			this->_pollitems[clientSocket]->pollState = READ_REQ_BODY;
		else
			this->_pollitems[clientSocket]->pollState = READ_CGI_RESPONSE;
	}
	else if (request->isAutoIndex())	//	GET (index)
		this->_pollitems[clientSocket]->pollState = WRITE_TO_CLIENT;
	else		// GET (HTML)
	{
		_addStaticFileFd(request->getRealPath(), clientSocket);
		this->_pollitems[clientSocket]->pollState = READ_STATIC_FILE;
	}
}

void	WebServer::_startCGI( int clientSocket)
{
	HTTPrequest 	*request = this->_requests.at(clientSocket);
	HTTPresponse 	*response = this->_responses.at(clientSocket);
	CGI				*cgiPtr = nullptr;

	response->setIsCGI(true);
	cgiPtr = new CGI(*request);
	this->_cgi[clientSocket] = cgiPtr;
	this->_addConn(cgiPtr->getResponsePipe()[0], CGI_RESPONSE_PIPE, READ_CGI_RESPONSE);
	cgiPtr->run();
	if (request->hasBody()) 
	{
		response->setFileUpload(request->isFileUpload());
		this->_addConn(cgiPtr->getuploadPipe()[1], CGI_REQUEST_PIPE, WRITE_TO_CGI);
	}
}

void	WebServer::_addStaticFileFd( std::string const& fileName, int clientSocket)
{
	HTTPresponse	*response = this->_responses.at(clientSocket);
	int 			HTMLfd = open(fileName.c_str(), O_RDONLY);

	if (HTMLfd == -1)
		throw(HTTPexception({"resource", fileName, "not found", fileName}, 404));
	response->setHTMLfd(HTMLfd);
	_addConn(HTMLfd, STATIC_FILE, READ_STATIC_FILE);
}

void	WebServer::readStaticFiles( int staticFileFd )
{
	int 			socket = _getSocketFromFd(staticFileFd);
	HTTPresponse	*response = this->_responses.at(socket);

	if (this->_pollitems[staticFileFd]->pollType != STATIC_FILE)
		return ;
	response->readHTML();
	if (response->isDoneReadingHTML())
	{
		this->_emptyConns.push_back(staticFileFd);
		this->_pollitems[socket]->pollState = WRITE_TO_CLIENT;
	}
}

void	WebServer::readRequestBody( int clientSocket )
{
	HTTPrequest *request = this->_requests[clientSocket];
	if (request->getTmpBody() == "") {
		request->parseBody();
	}
}

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
				this->_pollitems[request->getSocket()]->pollState = READ_CGI_RESPONSE;
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
	// HTTPresponse	*response = nullptr;

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
		this->_emptyConns.push_back(cgiPipe);
		this->_pollitems[socket]->pollState = WRITE_TO_CLIENT;
	}
}

void	WebServer::writeToClients( int clientSocket )
{
	HTTPrequest 	*request = this->_requests.at(clientSocket);
	HTTPresponse 	*response = this->_responses.at(clientSocket);

	if (response->isParsingNeeded() == true)
		_parseResponse(clientSocket);
	response->writeContent();
	if (response->isDoneWriting())
	{
		if (request->isEndConn() == true)
			this->_emptyConns.push_back(clientSocket);
		else
		{
			_dropStructs(clientSocket);
			this->_pollitems[clientSocket]->pollState = READ_REQ_HEADER;
		}
	}
}

void	WebServer::_parseResponse( int clientSocket )
{
	HTTPrequest 	*request = this->_requests.at(clientSocket);
	HTTPresponse 	*response = this->_responses.at(clientSocket);

	if ((request->isCGI()) and (response->getStatusCode() < 400)) {
		CGI *cgi = this->_cgi.at(clientSocket);
		response->parseFromCGI(cgi->getResponse());
	}
	else
	{
		if ((request->isAutoIndex()) and (response->getStatusCode() < 400))
			response->readContentDirectory(request->getRealPath());
		response->setServName(_getHandler(request->getHost()).getPrimaryName());
		response->parseFromStatic();
	}
}

void	WebServer::redirectToErrorPage( int genericFd, int statusCode ) noexcept
{
	int				clientSocket = -1;
	HTTPresponse	*response = nullptr;
	HTTPrequest		*request = nullptr;
	t_path			HTMLerrPage;

	try
	{
		clientSocket = _getSocketFromFd(genericFd);
		response = this->_responses.at(clientSocket);	// drop current response, create a new one
		request = this->_requests.at(clientSocket);
		if ((this->_pollitems[genericFd]->pollType == STATIC_FILE) or
			(this->_pollitems[genericFd]->pollType == CGI_REQUEST_PIPE) or		// NB: pipes need to be closed differently see _dropConn()
			(this->_pollitems[genericFd]->pollType == CGI_RESPONSE_PIPE))
			this->_emptyConns.push_back(genericFd);
		response->errorReset(statusCode);
		if (statusCode == 500)
			throw(HTTPexception({"internal error"}, 500));
		HTMLerrPage = _getHTMLerrorPage(statusCode, request->getErrorPages());	// NB: change _getHTMLerrorPage() after validation is ok
		_addStaticFileFd(HTMLerrPage, clientSocket);
		this->_pollitems[clientSocket]->pollState = READ_STATIC_FILE;
	}
	catch(const HTTPexception& e1) {
		std::cerr << C_RED << e1.what() << '\n' << C_RESET;
		response->updateStatic500();
		this->_pollitems[clientSocket]->pollState = WRITE_TO_CLIENT;
	}
	catch(const std::exception& e2)
	{
		std::cerr << C_RED << "invalid fd or request/response not found\n" << C_RESET;
		this->_emptyConns.push_back(genericFd);
		if (clientSocket != -1)
			this->_emptyConns.push_back(clientSocket);
	}
}
