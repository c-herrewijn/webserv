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

WebServer::WebServer ( std::vector<ConfigServer> const& servers ) : _servers(servers)
{
	bool defServerFound = false;

	if (servers.empty() == true)
		throw(ServerException({"No Servers provided for configuration"}));
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
	for (auto item : this->_requests)
		delete item.second;
	for (auto item : this->_responses)
		delete item.second;
	for (auto item : this->_cgi)
		delete item.second;
	for (auto item : this->_pollfds)
	{
		shutdown(item.fd, SHUT_RDWR);
		close(item.fd);
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

}

void			WebServer::loop( void )
{
	int					nConn = -1;
	std::vector<int>	emptyConns;

	while (true)
	{
		nConn = poll(this->_pollfds.data(), this->_pollfds.size(), 0);
		if (nConn < 0)
		{
			if ((errno != EAGAIN) and (errno != EWOULDBLOCK))
				throw(ServerException({"poll failed"}));
		}
		// some notes:
		//		1. update reading for chunked requests: it is possibile un unchunk
		// 			the body directly while reading, intstead of storing it in a variable
		// 		2. because of fork, waitpid (no hang) should be checked here to terminate any child
		//		3. also during signal (children have to be killed?)
		//
		// markAllPollItemsAsActionable();
		//
		// all functions should loop through the _requests, but only act on certain states
		// handleNewClientConnections();  // state: READ_REQ_HEADERS
		// 							// CLIENT_CONNECTION (reading header only: )
		// 							// if CGI, run the program
		//
		// handleCGIRequest();			// state: FOREWARD_REQ_TO_CGI
		// 							// CLIENT_CONNECTION (reading body POLLIN),
		// 							// CGI_DATA_PIPE (writing body POLLOUT)
		//
		// handleCGIResponse();
		// 							// CGI_RESPONSE_PIPE (reading response) state: FORWARD_CGI_RESPONSE
		// 							// CGI_RESPONSE_PIPE (reading POLLIN),
		// 							// CLIENT_CONNECTION (writing POLLOUT)
		//
		// countStaticFileLength(); 	// state: DETERMINE_STATIC_FILE_LENGTH
		// 							// STATIC_FILE (reading file)
		//
		// forewardStaticFile()		// state: FOREWARD_STATIC_FILE
		// 							// STATIC_FILE (read)
		// 							// CLIENT_CONNECTION (write)

		// int initialNrPollFds = this->_pollfds.size();		NB: why not iterate also on the new elements?
		for(size_t i=0; i<this->_pollfds.size(); i++)
		{
			struct pollfd &iPollFd = this->_pollfds[i];
			t_PollItem &pollItem = this->_pollitems[iPollFd.fd];
			try {
				if (iPollFd.revents & POLLIN) {
					if (pollItem.pollState == WAITING_FOR_CONNECTION) {
						std::cerr << C_GREEN << "POLLIN - NEWCONNECTION - " << iPollFd.fd << C_RESET << std::endl;
						handleNewConnections(pollItem);
					}
					else if (pollItem.pollState == READ_REQ_HEADER) {
						std::cerr << C_GREEN << "POLLIN - READ_REQ_HEADER - " << iPollFd.fd << C_RESET << std::endl;
						readRequestHeaders(pollItem);
					}
					else if (pollItem.pollState == FORWARD_REQ_BODY_TO_CGI) {
						std::cerr << C_GREEN << "POLLIN - FORWARD_REQ_BODY_TO_CGI - " << iPollFd.fd << C_RESET << std::endl;
					}
					else if (pollItem.pollState == READ_CGI_RESPONSE) {
						std::cerr << C_GREEN << "POLLIN - READ_CGI_RESPONSE " << iPollFd.fd << C_RESET << std::endl;
						readCGIResponses(pollItem, emptyConns);
					}
					else if (pollItem.pollState == READ_STATIC_FILE) {
						std::cerr << C_GREEN << "POLLIN - READ_STATIC_FILE - " << iPollFd.fd << C_RESET << std::endl;
						readStaticFiles(pollItem, emptyConns);
					}
				}
				else if (iPollFd.revents & POLLOUT)
				{
					if (pollItem.pollState == FORWARD_REQ_BODY_TO_CGI) {
						;
					}
					else if (pollItem.pollState == WRITE_TO_CLIENT) {
						std::cerr << C_GREEN << "POLLOUT - WRITE_TO_CLIENT - " << iPollFd.fd << C_RESET << std::endl;
						writeToClients(pollItem, emptyConns);
					}
				}
				else if (iPollFd.revents & (POLLHUP | POLLERR | POLLNVAL))	// client-end side was closed / error / socket not valid
					emptyConns.push_back(iPollFd.fd);
			}
			catch (const ServerException& e) {
				std::cerr << e.what() << '\n';
				
				// error handling
				emptyConns.push_back(iPollFd.fd);
			}
			catch (const HTTPexception& e) {
				std::cerr << e.what() << '\n';
				HTTPresponse *response = this->_responses[iPollFd.fd];
				response->setStatusCode(e.getStatus());
				if (response->getStatusCode() != 500)
				{
					// mapping between error codes and HTML files (except 500)
					// set HTML opening the correspondent HTML file
					int HTMLfdError = open("path/to/html/static/error/file", O_RDONLY);
					response->setHTMLfd(HTMLfdError);
					_addConn(HTMLfdError, STATIC_FILE, READ_STATIC_FILE);
					pollItem.pollState = READ_STATIC_FILE;
				}
				else
				{
					std::cout << "here\n";
					pollItem.pollState = WRITE_TO_CLIENT;
				}
			}
		}
		while (emptyConns.empty() == false)
		{
			_dropConn(emptyConns.back());
			emptyConns.pop_back();
		}
	}
}

std::string		WebServer::getAddress( const struct sockaddr_storage *addr ) const noexcept
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

ConfigServer const&	WebServer::getHandler( std::string const& servName ) const noexcept
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
	return (getDefaultServer());
}

ConfigServer const&	WebServer::getDefaultServer( void ) const noexcept
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
		throw(ServerException({"failed listen on", this->getAddress(&hostIP)}));
	}
	std::cout << "listening on: " << this->getAddress(&hostIP) << "\n";
	this->_addConn(listenSocket, LISTENER, WAITING_FOR_CONNECTION);
}

void	WebServer::_addConn( int newSocket , fdType typePollItem, fdState statePollItem )
{
	if (newSocket == -1)
		throw(ServerException({"invalid file descriptor"}));
	this->_pollfds.push_back({newSocket, POLLIN | POLLOUT, 0});
	t_PollItem newPollitem = {newSocket, typePollItem, statePollItem, false};
	this->_pollitems.insert(std::pair(newSocket, newPollitem));
}

void	WebServer::_dropConn(int toDrop) noexcept
{
	shutdown(toDrop, SHUT_RDWR);
	close(toDrop);
	for (auto start=this->_pollfds.begin(); start != this->_pollfds.end(); start++)
	{
		if (start->fd == toDrop)
		{
			this->_pollfds.erase(start);
			break;
		}
	}
	this->_pollitems.erase(toDrop);
}

void	WebServer::handleNewConnections( t_PollItem& item )
{
	struct sockaddr_storage client;
	unsigned int 			sizeAddr = sizeof(client);
	int 					connfd = -1;

	connfd = accept(item.fd, (struct sockaddr *) &client, &sizeAddr);
	if (connfd == -1)
		throw(ServerException({"connection with", this->getAddress(&client), "failed"}));
	fcntl(connfd, F_SETFL, O_NONBLOCK);
	std::cout << "connected to " << this->getAddress(&client) << '\n';
	this->_addConn(connfd, CLIENT_CONNECTION, READ_REQ_HEADER);
}

void	WebServer::readRequestHeaders( t_PollItem& pollItem )
{
	HTTPrequest 	*request = nullptr;
	HTTPresponse	*response = nullptr;
	CGI 			*cgiPtr = nullptr;
	int 			HTMLfd = -1;

	request = new HTTPrequest;
	this->_requests.insert(std::pair<int, HTTPrequest*>(pollItem.fd, request));
	request->setSocket(pollItem.fd);
	request->parseHead();	// if 0 chars are read, the client has closed the connection
	request->setConfigServer(&this->getHandler(request->getHost()));
	// validation from configServer (chocko's validation)
	request->checkHeaders(1000000);	// has to be dynamic
	response = new HTTPresponse;
	this->_responses.insert(std::pair<int, HTTPresponse*>(pollItem.fd, response));
	response->setSocket(pollItem.fd);
	response->setServName(request->getConfigServer().getPrimaryName());
	if (request->isCGI()) {
		cgiPtr = new CGI(*request);
		this->_cgi.insert(std::pair<int, CGI*>(pollItem.fd, cgiPtr));
		this->_addConn(cgiPtr->getResponsePipe()[0], CGI_RESPONSE_PIPE, READ_CGI_RESPONSE);
		cgiPtr->run();
		if (request->hasBody()) {
			this->_addConn(cgiPtr->getuploadPipe()[1], CGI_DATA_PIPE, FORWARD_REQ_BODY_TO_CGI);
			pollItem.pollState = FORWARD_REQ_BODY_TO_CGI;
		}		
		else {
			pollItem.pollState = READ_CGI_RESPONSE;
		}
	}
	else
	{
		if (request->getPath() == "/")		// NB: needs to be dynamic
			HTMLfd = open("var/www/test.html", O_RDONLY);
		else if (request->getPath() == "/favicon.ico")
			HTMLfd = open("var/www/favicon.ico", O_RDONLY);
		else
			HTMLfd = open(request->getPath().c_str(), O_RDONLY);
		response->setHTMLfd(HTMLfd);
		_addConn(HTMLfd, STATIC_FILE, READ_STATIC_FILE);
		pollItem.pollState = READ_STATIC_FILE;
	}
}

void	WebServer::readStaticFiles( t_PollItem& currentPoll, std::vector<int>& emptyConns )
{
	HTTPresponse	*response = nullptr;

	for (auto& item : this->_responses)
	{
		if (item.second->getHTMLfd() == currentPoll.fd)
		{
			response = item.second;
			break ;
		}
	}
	if (response == nullptr)
		throw(ServerException({"response not found"}));
	response->readHTML();
	if (response->isDoneReadingHTML())
	{
		emptyConns.push_back(currentPoll.fd);
		this->_pollitems[response->getSocket()].pollState = WRITE_TO_CLIENT;
	}
}

void	WebServer::forwardRequestBodyToCGI( t_PollItem& item )
{
	(void) item ;
	; // todo
}

// void	WebServer::readCGIResponses_bk( t_PollItem& pollItem )
// {
// 	HTTPrequest *request = nullptr;
// 	CGI         *cgi = nullptr;
//
// 	for (auto& cgiMapItem : this->_cgi)
// 	{
// 		if (cgiMapItem.second->getResponsePipe()[0] == pollItem.fd)
// 		{
// 			cgi = cgiMapItem.second;
// 			request = this->_requests[cgi->getRequestSocket()];
// 			break;
// 		}
// 	}
// 	if (request == nullptr || cgi == nullptr)
// 	{
// 		if (request == nullptr)
// 			std::cerr << C_RED << "error: request not found" << C_RESET << std::endl;
// 		if (cgi == nullptr)
// 			std::cerr << C_RED << "error: cgi not found" << C_RESET << std::endl;
// 	}
// 	char buffer[DEF_BUF_SIZE];
// 	int readChars = read(pollItem.fd, buffer, DEF_BUF_SIZE);
// 	std::string newResponsePart(buffer, buffer + readChars);
// 	request->cgi->appendResponse(newResponsePart);
//
// 	// TODO: support partial reads, i.e. in case of very big CGI response
// 	if (true) // TODO keep pipe open for consequtive reads if needed
// 	{
// 		close(request->cgi->getResponsePipe()[0]);
// 		this->_pollitems[request->getSocket()].pollState = WRITE_TO_CLIENT;
// 	}
//
// }

void	WebServer::readCGIResponses( t_PollItem& pollItem, std::vector<int>& emptyConns )
{
	CGI		*cgi = nullptr;

	for (auto& cgiMapItem : this->_cgi)
	{
		if (cgiMapItem.second->getResponsePipe()[0] == pollItem.fd)
		{
			cgi = cgiMapItem.second;
			break;
		}
	}
	if (cgi == nullptr)
		throw(ServerException({"cgi not found"}));
	// NB: move this functionality inside CGI class?
	ssize_t	readChars = -1;
	char 	buffer[DEF_BUF_SIZE];
	bzero(buffer, DEF_BUF_SIZE);
	readChars = read(pollItem.fd, buffer, DEF_BUF_SIZE);
	if (readChars < 0)
		throw(CGIexception({"cgi pipe not available"}, 500));
	cgi->appendResponse(std::string(buffer, buffer + readChars));

	// TODO: support partial reads, i.e. in case of very big CGI response
	// a logic to understand when the whole CGI response is received is necessary
	// to know when to close the pipe connection
	if (true) // TODO keep pipe open for consequtive reads if needed
	{
		HTTPresponse *response = this->_responses[cgi->getRequestSocket()];
		response->parseFromCGI(cgi->getResponse());
		emptyConns.push_back(pollItem.fd);
		this->_pollitems[cgi->getRequestSocket()].pollState = WRITE_TO_CLIENT;
	}
}

void	WebServer::writeToClients( t_PollItem& pollItem, std::vector<int>& emptyConns )
{
	HTTPrequest 	*request = this->_requests[pollItem.fd];
	HTTPresponse 	*response = this->_responses[pollItem.fd];

	if (request->isCGI()) {
		CGI *cgi = this->_cgi[pollItem.fd];
		response->parseFromCGI(cgi->getResponse());
		delete (cgi);
		this->_cgi.erase(pollItem.fd);
	}
	else
		response->parseFromStatic();
	response->writeContent();
	if (response->isDoneWriting())
	{
		if (request->isEndConn())
			emptyConns.push_back(pollItem.fd);
		else
			this->_pollitems[pollItem.fd].pollState = READ_REQ_HEADER;
		delete (response);
		this->_responses.erase(pollItem.fd);
		delete (request);
		this->_requests.erase(pollItem.fd);
	}
}
