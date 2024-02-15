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

void			WebServer::loop( void )
{
	int	nConn = -1;
	HTTPresponse 	response;
	std::string		rawContent;

	while (true)
	{
		nConn = poll(this->_connfds.data(), this->_connfds.size(), 0);
		if (nConn < 0)
		{
			if ((errno != EAGAIN) and (errno != EWOULDBLOCK))
				throw(ServerException({"poll failed"}));
		}
		else if (nConn > 0)
		{
			for (size_t i=0; i<this->_connfds.size(); i++)
			{
				try {
					if (this->_connfds[i].revents & POLLIN)
					{
						if (_isListener(this->_connfds[i].fd) == true) // new connection
							_acceptConnection(this->_connfds[i].fd);
						else
						{
							rawContent = _readHead(this->_connfds[i].fd);
							if (rawContent.empty() == true)		// client closed connection
								throw(ServerException({"(empty) client closed the connection"}));
							response = _handleRequest(this->_connfds[i].fd, rawContent);
							response.setSocket(this->_connfds[i].fd);
							std::cout << response.toString();
						}
					}
					else if (this->_connfds[i].revents & POLLOUT)
					{
						_writeResponse(response);
						if (response.getStatusCode() != 200)
							_dropConn(this->_connfds[i--].fd);
					}
					if (this->_connfds[i].revents & (POLLHUP | POLLERR | POLLNVAL))	// client-end side was closed / error / socket not valid
					{
						std::cout << "POLLHUP | POLLERR | POLLNVAL\n";
						_dropConn(this->_connfds[i--].fd);
					}
				}
				catch(const ServerException& e) {
					std::cerr << e.what() << '\n';
					_dropConn(this->_connfds[i--].fd);
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
		throw(ServerException({"connection with", this->getAddress(&client), "failed"}));
	fcntl(connfd, F_SETFL, O_NONBLOCK);
	std::cout << "connected to " << this->getAddress(&client) << '\n';
	this->_addConn(connfd);
}

HTTPresponse	WebServer::_handleRequest( int connfd, std::string const& rawContent ) const
{
	std::string		strHead, strBody;
	size_t			endHeadPos = std::string::npos;
	int				status = 200;
	HTTPrequest 	request;
	Executor		executor;
	Server 			handler;

	endHeadPos = rawContent.find(HTTP_TERM);
	if (endHeadPos != std::string::npos)
	{
		endHeadPos += HTTP_TERM.size();
		strHead = rawContent.substr(0, endHeadPos);
		strBody = rawContent.substr(endHeadPos);
	}
	try {
		request.setSocket(connfd);
		request.parseHead(strHead);
		request.storeTmpBody(strBody)
		handler = getHandler(request.getHost());
	}
	catch (const HTTPexception& e) {
		std::cerr << e.what() << '\n';
		handler = getDefaultServer();
		status = e.getStatus();
	}
	executor.setHandler(handler);
	if (status == 200)
		return (executor.execRequest(request));
	else
		return (executor.createResponse(status, ""));
}

std::string		WebServer::_readHead( int fd ) const
{
	char		buffer[HEADER_BUF_SIZE + 1];
	ssize_t 	readChar = -1;
	std::string content;
	// struct pollfd	toListen;
	// int 			nConn = -1;

	// toListen.fd = fd;
	// toListen.events = POLLIN;
	// toListen.revents = 0;
	// nConn = poll(&toListen, 1, 0);
	// if (nConn < 0)
	// {
	// 	if ((errno != EAGAIN) and (errno != EWOULDBLOCK))
	// 		throw(ServerException({"poll failed"}));
	// }
	// else if (nConn == 0)
	// 	return ("");
	// else if (toListen.revents & POLLIN)
	// {
		while (true)
		{
			bzero(buffer, HEADER_BUF_SIZE + 1);
			readChar = recv(fd, buffer, HEADER_BUF_SIZE, 0);
			(void) readChar;
			// if (readChar < 0)
			// {
				// if ((errno == EAGAIN) or (errno == EWOULDBLOCK))
				// 	std::cout << "socket thinghy\n";
				// else
				// {
				// 	std::cout << "socket not available from read head\n";
					// throw(ServerException({"from reading head: socket not available"}));
				// }
			// }
			content += std::string(buffer);
			if (content.find(HTTP_TERM) != std::string::npos)
				break;
		}
	// }
	return (content);
}

std::string		WebServer::_readRemainingBody( int socket, size_t maxBodylength, size_t sizeBody) const
{
    ssize_t     	lenToRead, readChar=-1;
    char        	*buffer = nullptr;
	std::string		body;

	if (maxBodylength == 0)
		lenToRead = std::numeric_limits<ssize_t>::max();
	else if (maxBodylength < sizeBody)
		throw(RequestException({"body length is longer than maximum allowed"}, 413));
	else if (maxBodylength == sizeBody)
		return ("");
	else
		lenToRead = maxBodylength - sizeBody;
	buffer = new char[lenToRead + 2];
	bzero(buffer, lenToRead + 2);
	readChar = recv(socket, buffer, lenToRead + 1, 0);
	body = buffer;
	delete [] buffer;
	if ((maxBodylength != 0) and (readChar > (ssize_t) lenToRead))
		throw(RequestException({"body length is longer than maximum allowed"}, 413));
	return (body);
}

void			WebServer::_writeResponse( HTTPresponse const& resp ) const
{
	std::string 	toWrite;
	size_t			start=0, len=resp.toString().size();
	ssize_t 		written=0;
	// struct pollfd	toListen;
	// int 			nConn = -1;

	// toListen.fd = resp.getSocket();
	// toListen.events = POLLOUT;
	// toListen.revents = 0;
	// nConn = poll(&toListen, 1, 0);
	// if (nConn < 0)
	// {
	// 	if ((errno != EAGAIN) and (errno != EWOULDBLOCK))
	// 		throw(ServerException({"poll failed"}));
	// }
	// else if (nConn == 0)
	// 	return ;
	// else if (toListen.revents & POLLOUT)
	// {
		while (start < resp.toString().size())
		{
			toWrite = resp.toString().substr(start, len);
			written = send(resp.getSocket(), toWrite.c_str(), len, 0);
			if (written < -1)
			{
				// if ((errno == EAGAIN) or (errno == EWOULDBLOCK))
				// 	std::cout << "socket thinghy\n";
				// else
				// {
				// 	std::cout << "socket not available from write\n";
					throw(ServerException({"from writing: socket not available"}));
				// }
			}
			start += written;
			len -= written;
		}
	// }
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
	for (size_t i=0; i<this->_connfds.size(); i++)
	{
		if ((this->_connfds[i].fd == toDrop) or (toDrop == -1))
		{
			shutdown(this->_connfds[i].fd, SHUT_RDWR);
			close(this->_connfds[i].fd);
			this->_connfds.erase(this->_connfds.begin() + i);
			if (this->_isListener(this->_connfds[i].fd) == true)
				this->_listeners.erase(this->_connfds[i].fd);
			if (toDrop != -1)
				break;
			i--;
		}
	}
}
