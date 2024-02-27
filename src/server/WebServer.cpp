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

// WebServer::addRequestIfNew(int socket)
// {

// }

void			WebServer::loop( void )
{
	int				nConn = -1;
	HTTPrequest 	*request;
	HTTPresponse	response;

	while (true)
	{
		nConn = poll(this->_pollfds.data(), this->_pollfds.size(), 0);
		if (nConn < 0)
		{
			if ((errno != EAGAIN) and (errno != EWOULDBLOCK))
				throw(ServerException({"poll failed"}));
		}
		else if (nConn > 0)
		{
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

			for (size_t i=0; i<this->_pollfds.size(); i++)
			{
				if (nConn == 0)
					break ;
				try {
					if (this->_pollfds[i].revents & POLLIN) {
						t_PollItem &current = _getPollItem(this->_pollfds[i].fd);
						if (current.pollState == WAITING_FOR_CONNECTION) {
							handleNewConnections(current);
						}
						else if (current.pollState == READ_REQ_HEADER) {


							// this->_requests.push_back(executor);
							request = new HTTPrequest;
							try {
								// RequestExecutor executor(current.fd);
								request->readHead(current.fd);
								request->setConfigServer(&this->getHandler(request->getHost()));
								// executor.setRequest(&request);
								// validation from configServer (chocko's validation)
								request->checkHeaders(1000000);
								if (request->isCGI()) {
									if (request->hasBody())
										current.pollState = FORWARD_REQ_BODY_TO_CGI;
									else
										current.pollState = READ_CGI_RESPONSE;
								}
								else
									current.pollState = READ_STATIC_FILE;
								this->_requests.push_back(request);
								// is static file -> status = READ_STATIC_FILE
								// else is CGI (POST and DELETE) -> is there a body?
								//				yes: status = FOREWARD_REQ_BODY_TO_CGI
								//				no: status = READ_CGI_RESPONSE




// Non-CGI END of headers read without “content-length"	READ_STATIC_FILE
// Non-CGI END of headers read with “content-length"		WRITE_TO_CLIENT			error case, discard request body by reading it all
// CGI END of headers read without “content-length"		READ_CGI_RESPONSE		run the CGI
// CGI END of headers read with “content-length"			FOREWARD_REQ_BODY_TO_CGI	run the CGI



							}
							catch (const HTTPexception& e) {
								// TODO
							}




							// read max size of header (8192 bytes)
							// parse header
							// store overshoot

							// replace this logic
							response = _handleRequest(this->_pollfds[i].fd);
							response.writeContent(this->_pollfds[i].fd);
							if (response.getStatusCode() != 200)
								_dropConn(this->_pollfds[i--].fd);
						}
						else if (current.pollState == FORWARD_REQ_BODY_TO_CGI) {
							// replace this logic
							response = _handleRequest(this->_pollfds[i].fd);
							response.writeContent(this->_pollfds[i].fd);
							if (response.getStatusCode() != 200)
								_dropConn(this->_pollfds[i--].fd);
						}
						else if (current.pollState == READ_CGI_RESPONSE) {
							// replace this logic
							response = _handleRequest(this->_pollfds[i].fd);
							response.writeContent(this->_pollfds[i].fd);
							if (response.getStatusCode() != 200)
								_dropConn(this->_pollfds[i--].fd);
						}
						else if (current.pollState == READ_STATIC_FILE) {
							// replace this logic
							response = _handleRequest(this->_pollfds[i].fd);
							response.writeContent(this->_pollfds[i].fd);
							if (response.getStatusCode() != 200)
								_dropConn(this->_pollfds[i--].fd);
						}
						nConn--;
					}
					else if (this->_pollfds[i].revents & POLLOUT)
					{
						t_PollItem &current = _getPollItem(this->_pollfds[i].fd);

						if (current.pollState == FORWARD_REQ_BODY_TO_CGI) {
							// replace this logic
							response = _handleRequest(this->_pollfds[i].fd);
							response.writeContent(this->_pollfds[i].fd);
							if (response.getStatusCode() != 200)
								_dropConn(this->_pollfds[i--].fd);
						}
						else if (current.pollState == WRITE_TO_CLIENT) {
							// replace this logic
							response = _handleRequest(this->_pollfds[i].fd);
							response.writeContent(this->_pollfds[i].fd);
							if (response.getStatusCode() != 200)
								_dropConn(this->_pollfds[i--].fd);
						}

						// ...
						nConn--;
					}
					// if (this->_pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL))	// client-end side was closed / error / socket not valid
					// 	_dropConn(this->_pollfds[i--].fd);

					// 	readRequestHeaders(current);
					// 	readStaticFiles(current);
					// 	forwardRequestBodyToCGI(current);
					// 	readCGIResponses(current);
					// 	writeToClients(current);
				}
				catch(const ServerException& e) {
					std::cerr << e.what() << '\n';
					_dropConn(this->_pollfds[i--].fd);
				}
			}
		}
	}
	_dropConn();
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

ConfigServer const&	WebServer::getHandler( std::string const& servName ) const
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

ConfigServer const&	WebServer::getDefaultServer( void ) const
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

HTTPresponse	WebServer::_handleRequest( int connfd )
{
	HTTPrequest 	request;
	HTTPresponse	response;

	// this->_requests.push_back(executor);
	try {
		// RequestExecutor executor(connfd);
		request.readHead(connfd);
		request.setConfigServer(&this->getHandler(request.getHost()));
		response = request.execRequest();
	}
	catch (const HTTPexception& e) {
		std::cerr << e.what() << '\n';
		response.parseFromStatic(e.getStatus(), getDefaultServer().getPrimaryName(), "");
	}
	return (response);
}

void			WebServer::_addConn( int newSocket , fdType typePollItem, fdState statePollItem ) noexcept
{
	if (newSocket != -1)
	{
		this->_pollfds.push_back({newSocket, POLLIN | POLLOUT, 0});
		this->_pollitems.push_back({newSocket, typePollItem, statePollItem, false});
	}
}

void			WebServer::_dropConn(int toDrop) noexcept
{
	for (size_t i=0; i<this->_pollfds.size(); i++)
	{
		if ((this->_pollfds[i].fd == toDrop) or (toDrop == -1))
		{
			shutdown(this->_pollfds[i].fd, SHUT_RDWR);
			close(this->_pollfds[i].fd);
			this->_pollfds.erase(this->_pollfds.begin() + i);
			if (toDrop != -1)
				break;
			i--;
		}
	}
}

t_PollItem&	WebServer::_getPollItem( int value )
{
	for (auto &item : this->_pollitems)
	{
		if (item.fd == value)
			return (item);
	}
	throw(ServerException({"socket not found (this should not happen)"}));
}

void	WebServer::handleNewConnections( PollItem& item )
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

void	WebServer::readRequestHeaders( PollItem& item )
{
	(void) item ;
	; // todo
}

void	WebServer::readStaticFiles( PollItem& item )
{
	(void) item ;
	; // todo
}

void	WebServer::forwardRequestBodyToCGI( PollItem& item )
{
	(void) item ;
	; // todo
}

void	WebServer::readCGIResponses( PollItem& item )
{
	(void) item ;
	; // todo
}

void	WebServer::writeToClients( PollItem& item )
{
	(void) item ;
	; // todo
}
