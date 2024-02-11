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

WebServer::WebServer ( std::vector<Server> const& servers ) : _servers(servers)
{
	bool defServerFound = false;

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
	while(this->_connfds.empty() == false)
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

// NB: refine POLLOUT
// NB: re-add errno checking after poll() you silly goose!
void			WebServer::loop( void )
{
	int				nConn;
	HTTPresponse 	response;
	std::stack<int>	toDrop;

	while (true)
	{
		nConn = poll(this->_connfds.data(), this->_connfds.size(), MAX_TIMEOUT);
		if (nConn == -1)
			throw(ServerException({"poll failed"}));
		else if (nConn == 0)		// timeout (NB: right?)
			break;
		for (size_t i=0; i<this->_connfds.size(); i++)
		{
			if (this->_connfds[i].revents & POLLIN)
			{
				if (_isListener(this->_connfds[i].fd) == true) // new connection
					_acceptConnection(this->_connfds[i].fd);
				else
					response = _handleRequest(this->_connfds[i].fd);
			}
			if (this->_connfds[i].revents & POLLOUT)
			{
				_writeResponse(this->_connfds[i].fd, response.toString());
				if (response.getStatusCode() != 200)
					toDrop.push(this->_connfds[i].fd);
			}
			if ((this->_connfds[i].revents & POLLHUP) or (this->_connfds[i].revents & POLLERR))	// client-end side was closed
				toDrop.push(this->_connfds[i].fd);
		}
		while (toDrop.empty() == false)
		{
			_dropConn(toDrop.top());
			toDrop.pop();
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

// NB: the logic to find the handler relies only on the server name?
Server const&	WebServer::getHandler( std::string const& servName ) const
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

Server const&	WebServer::getDefaultServer( void ) const
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
		throw(ServerException({"no available IP host found for", hostname, "port", port}));
	}
	memmove(&hostIP, tmp->ai_addr, std::min(sizeof(struct sockaddr), sizeof(struct sockaddr_storage)));
	freeaddrinfo(list);
	if (listen(listenSocket, BACKLOG) != 0)
	{
		close(listenSocket);
		throw(ServerException({"failed listen on", this->getAddress(&hostIP)}));
	}
	std::cout << "listening on: " << this->getAddress(&hostIP) << "\n";
	this->_addConn(listenSocket);
	this->_listeners.insert(listenSocket);
}

void			WebServer::_acceptConnection( int listener )
{
	struct sockaddr_storage 	client;
	unsigned int 				sizeAddr = sizeof(client);
	int							connfd = -1;

	connfd = accept(listener, (struct sockaddr *) &client, &sizeAddr);
	if (connfd == -1)
		std::cout << "failed connection with: " << this->getAddress(&client) << '\n';
	else
		std::cout << "connected to " << this->getAddress(&client) << '\n';
	this->_addConn(connfd);
}

// NB: rewrite it better?
HTTPresponse	WebServer::_handleRequest( int connfd ) const
{
	std::string		strHead, partialBody;
	int				status = 200;
	HTTPrequest 	request;
	HTTPresponse 	response;
	Executor		executor;
	Server 			handler;
	
	try {
		_readHead(connfd, strHead, partialBody);
		request.parseHead(strHead);
	}
	catch (const HTTPexception& e) {
		std::cerr << e.what() << '\n';
		status = 400;
	}
	catch (const ServerException& e) {
		std::cerr << e.what() << '\n';
		status = 500;
	}
	if (status == 200)
	{
		if (partialBody.empty() == false)
			request.setTmpBody(partialBody);
		handler = getHandler(request.getHost());
	}
	else
		handler = getDefaultServer();
	executor.setHandler(handler);
	if (status == 200)
	{
		try {
			executor.storeRemainingBody(request, connfd);
			response = executor.execRequest(request);
		}
		catch (const HTTPexception& e) {
			std::cerr << e.what() << '\n';
			status = 400;
		}
		catch (const ServerException& e) {
			std::cerr << e.what() << '\n';
			status = 500;
		}
		catch (const ExecException& e) {
			std::cerr << e.what() << '\n';
			// status = 500;
		}
	}
	else
		response = executor.createResponse(status, "");
	return (response);
}

void			WebServer::_readHead( int fd, std::string& strHead, std::string& strBody) const
{
	char	buffer[HEADER_BUF_SIZE + 1];
	ssize_t readChar = -1;
	size_t	endHeadPos = std::string::npos;

	while (true)
	{
		bzero(buffer, HEADER_BUF_SIZE + 1);
		readChar = recv(fd, buffer, HEADER_BUF_SIZE, 0);
		if (readChar < 0)
			throw(ServerException({"Socket unavailable"}));
		endHeadPos = std::string(buffer).find(HTTP_TERM);
		if (endHeadPos != std::string::npos)
		{
			endHeadPos += HTTP_TERM.size();
			strHead += std::string(buffer).substr(0, endHeadPos);
			strBody = std::string(buffer).substr(endHeadPos);
			break;
		}
		else if (readChar < HEADER_BUF_SIZE)
			throw(HTTPexception({"no header terminator"}));
		else
			strHead += std::string(buffer);
	}
}

void			WebServer::_writeResponse( int fd, std::string const& content) const
{
	std::string toWrite;
	size_t	start=0, len=content.size();
	ssize_t written=0;

	while (start < content.size())
	{
		toWrite = content.substr(start, len);
		written = send(fd, toWrite.c_str(), len, 0);
		if (written < -1)
			throw(ServerException({"failed writing on client socket"}));
		start += written;
		len -= written;
	}
}

bool			WebServer::_isListener( int socket ) const
{
	return (this->_listeners.find(socket) != this->_listeners.end());
}

void			WebServer::_addConn( int newSocket ) noexcept
{
	struct pollfd	newfd;

	if (newSocket != -1)
	{
		newfd.fd = newSocket;
		newfd.events = POLLIN | POLLOUT;
		newfd.revents = 0;
		this->_connfds.push_back(newfd);
	}
}

void			WebServer::_dropConn(int toDrop) noexcept
{
	int currSocket;

	for (auto it=this->_connfds.begin(); it!=this->_connfds.end(); it++)
	{
		currSocket = (*it).fd;
		if ((toDrop == -1) or (currSocket == toDrop))
		{
			shutdown(currSocket, SHUT_RDWR);
			close(currSocket);
			this->_connfds.erase(it);
			if (this->_isListener(currSocket) == true)
				this->_listeners.erase(currSocket);
			if (toDrop != -1)
				break;
		}
	}
}
