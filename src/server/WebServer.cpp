#include "WebServer.hpp"

WebServer::WebServer( t_serv_list const& servers )
{
	std::vector<Listen>	distinctListeners;

	if (servers.empty() == true)
		throw(ServerException({"no Servers provided for configuration"}));
	this->_servers = servers;
	for (auto const& server : this->_servers)
	{
		for (auto const& address : server.getListens())
		{
			auto isPresent = std::find(distinctListeners.begin(), distinctListeners.end(), address);
			if ( isPresent == distinctListeners.end() )
				distinctListeners.push_back(address);
		}
	}
	for (auto const& listener : distinctListeners)
	{
		try {
			this->_listenTo(listener.getIpString(), listener.getPortString());
		}
		catch (const ServerException& e) {
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

void	WebServer::run( void )
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
	this->_addConn(listenSocket, LISTENER, WAITING_FOR_CONNECTION, hostname, port);
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
			std::cout << C_RED << "UNEXPECTED POLLIN - " << readFd << " - STATE: " << this->_pollitems[readFd]->pollState << C_RESET << std::endl;
			break;
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
			// std::cout << C_RED << "UNEXPECTED POLLOUT - " << writeFd << " - STATE: " << this->_pollitems[writeFd]->pollState << C_RESET << std::endl;
			break;
	}
}

void	WebServer::_addConn( int newSocket , fdType typePollItem, fdState statePollItem, std::string const& ip, std::string const& port )
{
	t_PollItem *newPollitem = nullptr;

	if (newSocket == -1)
		throw(ServerException({"invalid file descriptor"}));
	this->_pollfds.push_back({newSocket, POLLIN | POLLOUT, 0});
	newPollitem = new PollItem;
	newPollitem->fd = newSocket;
	newPollitem->pollType = typePollItem;
	newPollitem->pollState = statePollItem;
	newPollitem->IPaddr = ip;
	newPollitem->port = port;
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

std::string	WebServer::_getAddress( const struct sockaddr_storage *addr ) const noexcept
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

t_serv_list	WebServer::_getServersFromIP( std::string const& ip, std::string const& port) const noexcept
{
	t_serv_list	matchingServers;

	for (auto const& server : this->_servers)
	{
		for (auto const& address : server.getListens())
		{
			if ((address.getIpString() == ip) and (address.getPortString() == port))
			{
				matchingServers.push_back(server);
				break ;
			}
		}
	}
	return (matchingServers);
}

t_path	WebServer::_getDefErrorPage( int statusCode ) const
{
	for (auto const& dir_entry : std::filesystem::directory_iterator{SERVER_DEF_PAGES})
	{
		if (dir_entry.path().stem() == std::to_string(statusCode))
			return (dir_entry.path());
	}
	return ("/");
}

void	WebServer::handleNewConnections( int listenerFd )
{
	struct sockaddr_storage client;
	unsigned int 			sizeAddr = sizeof(client);
	int 					connFd = -1;

	connFd = accept(listenerFd, (struct sockaddr *) &client, &sizeAddr);
	if (connFd == -1)
		std::cerr << C_RED  << "connection with: " << this->_getAddress(&client) << " failed" << C_RESET << '\n';
	else
	{
		fcntl(connFd, F_SETFL, O_NONBLOCK);
		this->_addConn(connFd, CLIENT_CONNECTION, READ_REQ_HEADER, this->_pollitems[listenerFd]->IPaddr, this->_pollitems[listenerFd]->port);
		std::cout << C_GREEN << "connected to " << this->_getAddress(&client) << C_RESET << '\n';
	}
}

void	WebServer::readRequestHeaders( int clientSocket )
{
	HTTPrequest 	*request = nullptr;
	HTTPresponse	*response = nullptr;
	CGI				*cgi = nullptr;
	fdState			nextStatus;

	if (this->_requests[clientSocket] == nullptr)
		this->_requests[clientSocket] = new HTTPrequest(clientSocket, _getServersFromIP(this->_pollitems[clientSocket]->IPaddr, this->_pollitems[clientSocket]->port));
	request = this->_requests[clientSocket];
	request->parseHead();
	if (request->isDoneReadingHead() == false)
		return;
	response = new HTTPresponse(request->getSocket(), request->getStatusCode(), request->getType());
	this->_responses[clientSocket] = response;
	response->setTargetFile(request->getRealPath());
	if (request->isCGI())
	{
		cgi = new CGI(*request);
		if (request->isFastCGI() == true) {
			close(cgi->getUploadPipe()[0]);
			close(cgi->getUploadPipe()[1]);
		}
		this->_addConn(cgi->getResponsePipe()[0], CGI_RESPONSE_PIPE_READ_END, READ_CGI_RESPONSE);
		if (request->isFileUpload())
			this->_addConn(cgi->getUploadPipe()[1], CGI_REQUEST_PIPE_WRITE_END, WRITE_TO_CGI);
		this->_cgi[clientSocket] = cgi;
		cgi->run();
	}
	else if (request->isStatic())
		_addConn(response->getHTMLfd(), STATIC_FILE, READ_STATIC_FILE);
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
	int socket = _getSocketFromFd(cgiPipe);
	HTTPrequest *request = this->_requests[socket];
	ssize_t		readChars = -1;
	
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

void	WebServer::readCGIResponses( int cgiPipe )
{
	int 	socket = _getSocketFromFd(cgiPipe);
	CGI		*cgi = this->_cgi.at(socket);
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
		if (response->isCGI())
		{
			CGI *cgi = this->_cgi.at(clientSocket);
			response->parseFromCGI(cgi->getResponse());
		}
		else
		{
			if (response->isAutoIndex())
				response->listContentDirectory(request->getRealPath());
			response->parseFromStatic(request->getServName());
		}
	}
	response->writeContent();
	if (response->isDoneWriting() == false)
		return ;
	else if ((request->isEndConn() == true) or (request->getStatusCode() == 444))		// NGINX custom behaviour, if code == 444 connection is closed as well
		this->_emptyConns.push_back(clientSocket);
	else
	{
		_dropStructs(clientSocket);
		this->_pollitems[clientSocket]->pollState = READ_REQ_HEADER;
	}
}

void	WebServer::redirectToErrorPage( int genericFd, int statusCode ) noexcept
{
	int				clientSocket = _getSocketFromFd(genericFd);
	HTTPrequest		*request = this->_requests[clientSocket];
	HTTPresponse	*response = nullptr;
	t_path			HTMLerrPage;

	if (this->_pollitems[genericFd]->pollType > CLIENT_CONNECTION)	// when genericFd refers to a pipe or a static file
		this->_emptyConns.push_back(genericFd);
	if (this->_responses[clientSocket] == nullptr)
		this->_responses[clientSocket] = new HTTPresponse(request->getSocket(), statusCode);
	response = this->_responses[clientSocket];
	try {
		request->updateErrorCode(statusCode);
		response->errorReset(request->getStatusCode(), false);
		HTMLerrPage = request->getRealPath();
	}
	catch(const RequestException& e1) {
		std::cerr << C_RED << e1.what() << '\n' << C_RESET;
		HTMLerrPage = _getDefErrorPage(e1.getStatus());
		if (HTMLerrPage == "/")
		{
			response->errorReset(500, true);
			this->_pollitems[clientSocket]->pollState = WRITE_TO_CLIENT;
			return ;
		}
	}
	response->errorReset(statusCode, false);
	response->setTargetFile(HTMLerrPage);
	_addConn(response->getHTMLfd(), STATIC_FILE, READ_STATIC_FILE, "", "");
	this->_pollitems[clientSocket]->pollState = READ_STATIC_FILE;
}
