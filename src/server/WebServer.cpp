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

// NB: if an error occurs while CGI is running (fileupload) the cgi has to be stopped
void	WebServer::run( void )
{
	int				nConn = -1;
	struct pollfd 	pollfdItem;
	// int count = 0;
	// int oldnConn = 0;

	while (true)
	{
		nConn = poll(this->_pollfds.data(), this->_pollfds.size(), 0);
		// count++;
		// if (count == 1000000 || oldnConn != nConn) {
		// 	oldnConn = nConn;
		// 	std::cout << "polling...  nr poll items = " << nConn << "; "<< std::endl;
		// 	count = 0;
		// }

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
				if ((pollfdItem.revents & POLLIN) && (this->_pollitems[pollfdItem.fd]->pollType != CGI_RESPONSE_PIPE_READ_END))
					_readData(pollfdItem.fd);
				// POLLERR is expected when upload pipe is closed by CGI script
				if ((pollfdItem.revents & POLLOUT) && !(pollfdItem.revents & POLLERR))
					_writeData(pollfdItem.fd);
				if (pollfdItem.revents & (POLLHUP | POLLERR | POLLNVAL)) 	// client-end side was closed / error / socket not valid
				{
					if ((pollfdItem.revents & POLLHUP)
						&& (this->_pollitems[pollfdItem.fd]->pollType == CGI_RESPONSE_PIPE_READ_END))
					{
						if (this->_pollitems[pollfdItem.fd]->pollState == WAIT_FOR_CGI) {
							std::cout << C_GREEN  << "CGI Process finished - " << pollfdItem.fd << C_RESET << std::endl;
							this->_pollitems[pollfdItem.fd]->pollState = READ_CGI_RESPONSE;
						}
						_readData(pollfdItem.fd);
					}
					else {
						std::string errStr;
						if (pollfdItem.revents & POLLERR)
							errStr = "POLLERR";
						if (pollfdItem.revents & POLLNVAL)
							errStr = "POLLNVAL";
						if (pollfdItem.revents & POLLHUP)
							errStr = "POLLHUP";
						std::cout << C_RED << "fd: " << pollfdItem.fd << " client-end side was closed: " << errStr << C_RESET << std::endl;
						this->_emptyConns.push_back(pollfdItem.fd);
					}
				}
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
				std::cout << C_RED << e.what()  << C_RESET << '\n';
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

		case WAIT_FOR_CGI:
			// don't read yet, first wait for CGI to be done
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
			std::cout << C_RED << "UNEXPECTED POLLIN - " << readFd << " - state of pollitem: " << this->_pollitems[readFd]->pollState << C_RESET << std::endl;
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
	if ((this->_pollitems[toDrop]->pollType == CGI_REQUEST_PIPE_WRITE_END) or
		(this->_pollitems[toDrop]->pollType == CGI_RESPONSE_PIPE_READ_END))
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
	if (this->_pollitems[fd]->pollType == CGI_REQUEST_PIPE_WRITE_END)
	{
		for (auto& item : this->_cgi)
		{
			if (item.second->getUploadPipe()[1] == fd)
				return (item.second->getRequestSocket());
		}
	}
	else if (this->_pollitems[fd]->pollType == CGI_RESPONSE_PIPE_READ_END)
	{
		for (auto& item : this->_cgi)
		{
			if (item.second->getResponsePipe()[0] == fd)
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
	throw(std::out_of_range("invalid file descriptor or not found:"));	// entity not found, this should not happen
}

// NB: building absolute path?
t_path	WebServer::_getHTMLerrorPage( int statusCode, HTTPrequest *request ) const
{
	try {
		if (request->isDoneReadingHead() == false)	// fail occured even before validation, so no error pages, skipping directly to server ones
			throw(std::out_of_range(""));
		return (request->getErrorPages().at(statusCode));
	}
	catch(const std::out_of_range& e1)
	{
		try {
			return (_getHandler(request->getHost()).getParams().getErrorPages().at(statusCode));
		}
		catch(const std::out_of_range& e2) {
			try {
				return (_getDefaultHandler().getParams().getErrorPages().at(statusCode));
			}
			catch(const std::out_of_range& e3) {
				for (auto const& dir_entry : std::filesystem::directory_iterator{HTML_ERROR_FOLDER})
				{
					if (dir_entry.path().stem() == std::to_string(statusCode))
						return (dir_entry.path());
				}
				throw(ServerException({"absolutely no HTML found for code:", std::to_string(statusCode)}));
			}
		}
	}
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
	HTTPresponse 	*response = nullptr;
	CGI				*cgiPtr = nullptr;
	int				HTMLfd = -1;
	fdState			nextStatus = READ_REQ_HEADER;

	if (!this->_requests[clientSocket])
		this->_requests[clientSocket] = new HTTPrequest(clientSocket);
	request = this->_requests[clientSocket];
	request->parseHead();
	if (request->isDoneReadingHead() == false)
		return ;
	request->validate(_getHandler(request->getHost()));
	this->_responses[clientSocket] = new HTTPresponse(clientSocket, request->getType(), request->getStatusFromValidation());
	if (request->isCGI())
	{
		cgiPtr = new CGI(*request);
		if (request->getType() == HTTP_FAST_CGI) {
			close(cgiPtr->getUploadPipe()[0]);
			close(cgiPtr->getUploadPipe()[1]);
		}
		this->_cgi[clientSocket] = cgiPtr;
		this->_addConn(cgiPtr->getResponsePipe()[0], CGI_RESPONSE_PIPE_READ_END, READ_CGI_RESPONSE);
		if (request->isFileUpload())
			this->_addConn(cgiPtr->getUploadPipe()[1], CGI_REQUEST_PIPE_WRITE_END, WRITE_TO_CGI);
		cgiPtr->run();
	}
	else if (request->isStatic())
	{
		response = this->_responses[clientSocket];
		HTMLfd = open(request->getRealPath().c_str(), O_RDONLY);
		_addConn(HTMLfd, STATIC_FILE, READ_STATIC_FILE);
		response->setHTMLfd(HTMLfd);
		if (request->isRedirection())
			response->setRedirectFile(request->getRealPath());
	}
	if (request->isAutoIndex())
		nextStatus = WRITE_TO_CLIENT;
	else if (request->isFastCGI())
		nextStatus = WAIT_FOR_CGI;
	else if (request->theresBodyToRead())
		nextStatus = READ_REQ_BODY;
	else if (request->isStatic())
		nextStatus = READ_STATIC_FILE;		
	else
		nextStatus = READ_CGI_RESPONSE;
	this->_pollitems[clientSocket]->pollState = nextStatus;
}

void	WebServer::readStaticFiles( int staticFileFd )
{
	int 			socket = _getSocketFromFd(staticFileFd);
	HTTPresponse	*response = this->_responses.at(socket);

	if (this->_pollitems[staticFileFd]->pollType != STATIC_FILE)
		return ;
	response->readHTML();
	if (response->isDoneReadingHTML() == false)
		return ;
	this->_emptyConns.push_back(staticFileFd);
	this->_pollitems[socket]->pollState = WRITE_TO_CLIENT;
}

void	WebServer::readRequestBody( int clientSocket )
{
	HTTPrequest *request = this->_requests.at(clientSocket);
	if (request->getTmpBody() == "")	// NB: this check is problematic in case of chunked requests
		request->parseBody();
}

void	WebServer::writeToCGI( int cgiPipe )
{
	HTTPrequest *request = nullptr;
	ssize_t		readChars = -1;
	for (auto& cgiMapItem : this->_cgi) {
		if (cgiMapItem.second->getUploadPipe()[1] == cgiPipe) {
			request = this->_requests[cgiMapItem.first];
			break;
		}
	}
	if (request != nullptr) {
		close(this->_cgi[request->getSocket()]->getUploadPipe()[0]); // close read end of cgi upload pipe
		std::string tmpBody = request->getTmpBody();
		if (tmpBody != "") {
			readChars = write(cgiPipe, tmpBody.data(), tmpBody.length());
			if (readChars < 0)
				throw(ServerException({"unavailable socket"}));
			request->setTmpBody("");

			// drop from pollList after writing is done
			if (request->isDoneReadingBody()) {
				close(cgiPipe); // close write end of cgi upload pipe
				this->_emptyConns.push_back(cgiPipe);
				this->_pollitems[request->getSocket()]->pollState = WAIT_FOR_CGI;
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

	cgi = this->_cgi.at(socket);
	ssize_t	readChars = -1;
	char 	buffer[HTTP_BUF_SIZE];
	bzero(buffer, HTTP_BUF_SIZE);
	readChars = read(cgiPipe, buffer, HTTP_BUF_SIZE);
	if (readChars < 0)
		throw(ServerException({"unavailable socket"}));
	cgi->appendResponse(std::string(buffer, buffer + readChars));

	if (readChars < HTTP_BUF_SIZE)
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
	{
		if ((request->isCGI()) and (response->getStatusCode() < 400)) 	// NB: check the second condition
		{
			CGI *cgi = this->_cgi.at(clientSocket);
			response->parseFromCGI(cgi->getResponse());
		}
		else
		{
			if ((request->isAutoIndex()) and (response->getStatusCode() < 400))	// NB: check the second condition
				response->listContentDirectory(request->getRealPath());
			response->setServName(_getHandler(request->getHost()).getPrimaryName());
			response->parseFromStatic();
		}
	}
	response->writeContent();
	if (response->isDoneWriting() == false)
		return ;
	if ((request->isEndConn() == true) or (response->getStatusCode() == 444))
		this->_emptyConns.push_back(clientSocket);
	else
	{
		_dropStructs(clientSocket);
		this->_pollitems[clientSocket]->pollState = READ_REQ_HEADER;
	}
}

void	WebServer::redirectToErrorPage( int genericFd, int statusCode ) noexcept
{
	int				clientSocket=-1, HTMLfd=-1;
	HTTPresponse	*response = nullptr;
	HTTPrequest		*request = nullptr;
	t_path			HTMLerrPage;

	if ((this->_pollitems[genericFd]->pollType == STATIC_FILE) or
		(this->_pollitems[genericFd]->pollType == CGI_REQUEST_PIPE_WRITE_END) or
		(this->_pollitems[genericFd]->pollType == CGI_RESPONSE_PIPE_READ_END))
		this->_emptyConns.push_back(genericFd);
	try {
		clientSocket = _getSocketFromFd(genericFd);
		request = this->_requests.at(clientSocket);
		response = this->_responses[clientSocket];
		if (this->_responses[clientSocket] != nullptr)
			response->errorReset(statusCode);
		else
		{
			response = new HTTPresponse(clientSocket, HTTP_STATIC, statusCode);
			this->_responses[clientSocket] = response;
		}
		if (response->getStatusCode() == 500)
			this->_pollitems[clientSocket]->pollState = WRITE_TO_CLIENT;
		else
		{
			HTMLerrPage = _getHTMLerrorPage(statusCode, request);
			HTMLfd = open(HTMLerrPage.c_str(), O_RDONLY);
			response->setHTMLfd(HTMLfd);
			_addConn(HTMLfd, STATIC_FILE, READ_STATIC_FILE);
			this->_pollitems[clientSocket]->pollState = READ_STATIC_FILE;
		}
	}
	catch(const HTTPexception& e1) {
		std::cerr << C_RED << e1.what() << '\n' << C_RESET;
		response->errorReset(500);
		this->_pollitems[clientSocket]->pollState = WRITE_TO_CLIENT;
	}
	catch(const ServerException& e2) {
		std::cerr << C_RED << e2.what() << '\n' << C_RESET;
		response->errorReset(500);
		this->_pollitems[clientSocket]->pollState = WRITE_TO_CLIENT;
	}
	catch(const std::out_of_range& e3) {
		std::cerr << C_RED << "invalid fd or request/response not found\n" << C_RESET;
		this->_emptyConns.push_back(genericFd);
		if (clientSocket != -1)
			this->_emptyConns.push_back(clientSocket);
	}
}
