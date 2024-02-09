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
	for (auto server :this->_servers)
	{
		for (auto listAddress : server.getListens())
		{
			auto isPresent = std::find(this->_listenAddress.begin(),this->_listenAddress.end(), listAddress);
			if ( isPresent == this->_listenAddress.end())
				this->_listenAddress.push_back(listAddress);
		}
	}
}

WebServer::~WebServer ( void ) noexcept
{
	while(this->_connfds.empty() == false)
		this->_dropConn();
}

void	WebServer::startListen( void )
{
	try {
		for (auto listAddress : this->_listenAddress)
			this->_listenTo(listAddress.getIpString(), listAddress.getPortString());
	}
	catch(const WebServerException& e) {
		std::cout << e.what() << '\n';
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
			std::cout << "failed to update socket, trying to bind anyway... \n";
		if (bind(listenSocket, tmp->ai_addr, tmp->ai_addrlen) == 0)
			break;
		close(listenSocket);
	}
	if (tmp == nullptr)
	{
		freeaddrinfo(list);
		throw(ServerException({"no available IP host found for", hostname, ":",port}));
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

void		WebServer::_acceptConnection( int listener )
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

// NB: refine POLLOUT
// NB: fix when closing the connection, maybe the for loop has to be different?
// NB: the connection ready for POLLIN needs to assigned to the proper server
// NB: re-add errno checking after poll() you silly goose!
void	WebServer::loop( void )
{
	int			nConn;
	int			exitStatus = 200;
	std::string bodyResp;

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
				{
					exitStatus = _handleRequest(this->_connfds[i].fd, bodyResp);
				}
			}
			if (this->_connfds[i].revents & POLLOUT)
			{
				try
				{
					// _writeSocket(this->_connfds[i].fd, HTTPresponse().exitStatus, bodyResp).toString());
				}
				catch(const std::exception& e) {
					std::cerr << e.what() << '\n';
					_dropConn(this->_connfds[i].fd);
				}
				if (exitStatus >= 400)
					_dropConn(this->_connfds[i].fd);
				// try
				// {
				// 	if (request.headers.at("Connection") == "close")
				// 		_dropConn(this->_connfds[i].fd);
				// }
				// catch(...) {}
			}
		}
	}
}

int	WebServer::_handleRequest( int connfd, std::string& body )
{
	HTTPrequest		request;
	HTTPresponse	response;
	int				reqStat = -1;
	std::string		strHead, strBody;
	
	try {
		reqStat = _readHead(connfd, strHead);
		if (reqStat != 200)
			return (reqStat);
		(void) body;
	}
	catch (RequestException const& err) {
		std::cout << err.what() << '\n';
		return (400);
	}
	catch (WebServerException const& err) {
		std::cout << err.what() << '\n';
		return (500);
	}
	return (reqStat);
}

std::string	WebServer::getAddress( const struct sockaddr_storage *addr ) const noexcept
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

void	WebServer::_dropConn(int toDrop) noexcept
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
			break;
		}
	}
}

void	WebServer::_addConn( int newSocket ) noexcept
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

bool	WebServer::_isListener( int socket ) const
{
	return (this->_listeners.find(socket) != this->_listeners.end());
}

int		WebServer::_readHead( int fd, std::string& strHead) const
{
	char			buffer[HEADER_MAX_SIZE + 1];
	ssize_t 		readChar = HEADER_MAX_SIZE;
	size_t			endHeadPos;
	int				statusRead = 200;
	
	while (true)
	{
		bzero(buffer, HEADER_MAX_SIZE + 1);
		readChar = recv(fd, buffer, HEADER_MAX_SIZE, 0);
		if (readChar < 0)
			throw(ServerException({"socket not available or empty"}));
		endHeadPos = std::string(buffer).find(HTTP_TERM);
		if (endHeadPos != std::string::npos)
		{
			endHeadPos += HTTP_TERM.size();
			if (lseek(fd, endHeadPos, SEEK_SET) < 0)
				throw(ServerException({"error resetting the cursor position"}));
			strHead += std::string(buffer).substr(0, endHeadPos);
			break;
		}
		else if (readChar < HEADER_MAX_SIZE)
		{
			statusRead = 400;
			break;
		}
		else
			strHead += std::string(buffer);
	}
	return (statusRead);
}

void	WebServer::_writeSocket( int fd, std::string const& content) const
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

// NB: how can i keep the information about which connection close()?
// NB: the whole function needs to be changed
// void	WebServer::_waitForChildren( void )
// {
// 	int 	statProc;
// 	pid_t	childProc;

// 	// if (this->_currentJobs.empty())
// 	// 	return ;
// 	childProc = waitpid(-1, &statProc, WNOHANG);
// 	if (childProc < 0)
// 	{
// 		if (errno != ECHILD)
// 			throw(ServerException({"error while terminating process"}));
// 	}
// 	else
// 	{
// 		// if (this->_currentJobs.erase(childProc) == 0)
// 		// 	throw(ServerException({"no child process found with id:", std::to_string(childProc).c_str()}));
// 		if (WEXITSTATUS(statProc) != 0)
// 		{
// 			std::cout << "failed to a request, closing connection";
// 			_dropConn();
// 		}
// 	}
// }
