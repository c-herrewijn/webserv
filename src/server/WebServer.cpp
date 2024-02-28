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
	this->_dropConn();
	for (auto item : this->_requests)
		delete item.second;
	for (auto item : this->_responses)
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
	HTTPrequest 		*request;
	HTTPresponse		*response;
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

		auto begin = this->_pollfds.begin();
		while (nConn > 0)
		{
			try {
				if (begin->revents & POLLIN) {
					t_PollItem &current = this->_pollitems[begin->fd];
					if (current.pollState == WAITING_FOR_CONNECTION)
						handleNewConnections(current);
					else if (current.pollState == READ_REQ_HEADER) {
						request = new HTTPrequest;
						request->setSocket(current.fd);
						request->parseHead();
						request->setConfigServer(&this->getHandler(request->getHost()));
						// validation from configServer (chocko's validation), the validation has to set the maxBodyLength!
						request->checkHeaders(1000000);	// has to be dynamic
						if (request->isCGI()) {
							if (request->hasBody())
								current.pollState = FORWARD_REQ_BODY_TO_CGI;
							else
								current.pollState = READ_CGI_RESPONSE;
						}
						else
						{
							response = new HTTPresponse;
							response->setSocket(current.fd);
							response->setServName(request->getConfigServer().getPrimaryName());
							int HTMLfd = open(request->getPath().c_str(), O_RDONLY);
							response->setHTMLfd(HTMLfd);
							_addConn(HTMLfd, STATIC_FILE, READ_STATIC_FILE);
							current.pollState = READ_STATIC_FILE;
							this->_responses.insert(std::pair<int, HTTPresponse*>(current.fd, response));
						}
						this->_requests.insert(std::pair<int, HTTPrequest*>(current.fd, request));
					}
					else if (current.pollState == FORWARD_REQ_BODY_TO_CGI) {
						// replace this logic
						// response = _handleRequest(begin->fd);
						// response.writeContent(begin->fd);
						// if (response.getStatusCode() != 200)
						// 	_dropConn(this->_pollfds[i--].fd);
					}
					else if (current.pollState == READ_CGI_RESPONSE) {
						// replace this logic
						// response = _handleRequest(begin->fd);
						// response.writeContent(begin->fd);
						// if (response.getStatusCode() != 200)
						// 	_dropConn(this->_pollfds[i--].fd);
					}
					else if (current.pollState == READ_STATIC_FILE) {
						readStaticFiles(current, emptyConns);
					}
					nConn--;
				}
				else if (begin->revents & POLLOUT)
				{
					t_PollItem &current = this->_pollitems[begin->fd];
					if (current.pollState == FORWARD_REQ_BODY_TO_CGI) {

					}
					else if (current.pollState == WRITE_TO_CLIENT) {
						response = this->_responses[current.fd];
						response->writeContent();
						if (response->isDoneWriting())
							emptyConns.push_back(current.fd);
					}
					nConn--;
				}
				// else if (this->_pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL))	// client-end side was closed / error / socket not valid
				// 	_dropConn(this->_pollfds[i--].fd);
			}
			catch(const ServerException& e) {
				std::cerr << e.what() << '\n';
				// error handling
				_dropConn(begin->fd);
			}
			begin++;
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

void			WebServer::_listenTo( std::string const& hostname, std::string const& port )
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
	this->_addConn(listenSocket, SERVER_SOCKET, WAITING_FOR_CONNECTION);
}

void			WebServer::_addConn( int newSocket , fdType typePollItem, fdState statePollItem )
{
	if (newSocket == -1)
		throw(ServerException({"invalid file descriptor"}));
	this->_pollfds.push_back({newSocket, POLLIN | POLLOUT, 0});
	t_PollItem newPollitem = {newSocket, typePollItem, statePollItem, false};
	this->_pollitems.insert(std::pair(newSocket, newPollitem));
}

void			WebServer::_dropConn(int toDrop) noexcept
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
	if (this->_requests.count(toDrop) > 0)
	{
		delete (this->_requests[toDrop]);
		this->_requests.erase(toDrop);
	}
	if (this->_responses.count(toDrop) > 0)
	{
		delete (this->_responses[toDrop]);
		this->_responses.erase(toDrop);
	}
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

void	WebServer::readRequestHeaders( t_PollItem& item )
{
	(void) item ;
	; // todo
}

void	WebServer::readStaticFiles( t_PollItem& currentPoll, std::vector<int>& emptyConns )
{
	HTTPresponse	*response;

	for (auto& item : this->_responses)
	{
		if (item.second->getHTMLfd() == currentPoll.fd)
		{
			response = item.second;
			break ;
		}
	}
	response->readHTML();
	if (response->isDoneReadingHTML() == true)
	{
		emptyConns.push_back(currentPoll.fd);
		response->parseStaticHTML(200);
		this->_pollitems[response->getSocket()].pollState = WRITE_TO_CLIENT;
	}
}

void	WebServer::forwardRequestBodyToCGI( t_PollItem& item )
{
	(void) item ;
	; // todo
}

void	WebServer::readCGIResponses( t_PollItem& item )
{
	(void) item ;
	; // todo
}

void	WebServer::writeToClients( t_PollItem& item )
{
	(void) item ;
	; // todo
}
