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

WebServer::~WebServer ( void ) noexcept
{
	while(this->_connfds.empty() == false)
		this->_dropConn();
}

void	WebServer::listenTo( std::string const& hostname, std::string const& port )
{
	struct addrinfo *tmp, *list, filter;
	struct sockaddr_storage	hostip;
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
	memmove(&hostip, tmp->ai_addr, std::min(sizeof(struct sockaddr), sizeof(struct sockaddr_storage)));
	freeaddrinfo(list);
	if (listen(listenSocket, BACKLOG) != 0)
		throw(ServerException({"failed listen on", this->getAddress(&hostip)}));
	std::cout << "listening on: " << this->getAddress(&hostip) << "\n";
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
					_writeSocket(this->_connfds[i].fd, HTTPbuilder::buildResponse(exitStatus, bodyResp).toString());
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
	std::string		stringRequest;
	
	try
	{
		stringRequest = _readSocket(connfd);
		request = HTTPparser::parseRequest(stringRequest);
		// std::cout << request.toString();
		reqStat = HTTPexecutor::execRequest(request, body);
		// std::cout << response.toString();
	}
	catch (ParserException const& err) {
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

WebServer::WebServer ( WebServer const& other ) noexcept
{
	// ofc shallow copy of port and IP attributes would be problematic because
	// of the cuncurrency of two servers accessing the same ip:port, if fact it 
	// makes no sense to create copies of servers
	(void) other;
}

WebServer& WebServer::operator=( WebServer const& other ) noexcept
{
	// see copy constructor
	(void) other;
	return (*this);
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

std::string	WebServer::_readSocket( int fd ) const
{
	char			buffer[HEADER_MAX_SIZE + 1];
	ssize_t 		readChar = HEADER_MAX_SIZE;
	std::string		stringRequest;

	while (readChar == HEADER_MAX_SIZE)
	{
		memset(buffer, 0, HEADER_MAX_SIZE + 1);		// NB: remove C functions!
		readChar = recv(fd, buffer, HEADER_MAX_SIZE, 0);
		if (readChar < 0)
			throw(ServerException({"socket not available or empty"}));
		stringRequest += buffer;
	}
	return (stringRequest);
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
void	WebServer::_waitForChildren( void )
{
	int 	statProc;
	pid_t	childProc;

	// if (this->_currentJobs.empty())
	// 	return ;
	childProc = waitpid(-1, &statProc, WNOHANG);
	if (childProc < 0)
	{
		if (errno != ECHILD)
			throw(ServerException({"error while terminating process"}));
	}
	else
	{
		// if (this->_currentJobs.erase(childProc) == 0)
		// 	throw(ServerException({"no child process found with id:", std::to_string(childProc).c_str()}));
		if (WEXITSTATUS(statProc) != 0)
		{
			std::cout << "failed to a request, closing connection";
			_dropConn();
		}
	}
}
