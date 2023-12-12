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

ServerException::ServerException( std::initializer_list<const char*> prompts) noexcept 
	: std::exception()
{
	this->_msg = "Webserver exception: ";
	for (const char *prompt : prompts)
		this->_msg += std::string(prompt) + " ";
}


WebServer::WebServer ( void )
{
	bzero(&this->_filter, sizeof(struct addrinfo));
	this->_filter.ai_flags = AI_PASSIVE;
	this->_filter.ai_family = AF_UNSPEC;
	this->_filter.ai_protocol = IPPROTO_TCP;
}

WebServer::~WebServer ( void ) noexcept
{
	while(this->_connfds.empty() == false)
		this->_dropConn();
}

struct addrinfo	WebServer::getFilter( void ) const noexcept
{
	return(this->_filter);
}

void	WebServer::setFilter( struct addrinfo const& newFilter ) noexcept
{
	this->_filter = newFilter;
}

void	WebServer::listenAt( const char* hostname, const char* port )
{
	struct addrinfo *tmp, *list;
	struct sockaddr_storage	hostip;
	int yes=1, listenSocket=-1;

	if (getaddrinfo(hostname, port, &this->_filter, &list) != 0)
		throw(ServerException({"failed to get addresses for ", hostname, ":",port}));
	for (tmp=list; tmp!=nullptr; tmp=tmp->ai_next)
	{
		listenSocket = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
		if (listenSocket == -1)
			continue;
		if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0)
			std::cout << "failed to update socket, trying to bind anyway... \n";
		if (bind(listenSocket, tmp->ai_addr, tmp->ai_addrlen) == 0)
			break;
		close(listenSocket);
	}
	if (tmp == nullptr)
	{
		freeaddrinfo(list);
		throw(ServerException({"no available IP host found for ", hostname, ":",port}));
	}
	memmove(&hostip, tmp->ai_addr, std::min(sizeof(struct sockaddr), sizeof(struct sockaddr_storage)));
	freeaddrinfo(list);
	if (listen(listenSocket, BACKLOG) != 0)
		throw(ServerException({"failed listen on", this->getAddress(&hostip).c_str()}));
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

void	WebServer::loop( void )
{
	int	nConn;

	while (true)
	{
		nConn = poll(this->_connfds.data(), this->_connfds.size(), MAX_TIMEOUT);
		
		if (nConn == -1)
			throw(ServerException({"poll failure"}));
		else if (nConn == 0)
			break;
		for (size_t i=0; i<this->_connfds.size(); i++)
		{
			if (this->_connfds[i].revents & POLLIN)
			{
				if (this->_isListener(this->_connfds[i].fd) == true) // new connection
					this->_acceptConnection(this->_connfds[i].fd);
				else
					this->_handleRequest(this->_connfds[i--].fd);
			}
			else if (this->_connfds[i].revents & POLLOUT) 
			{
				// send HTTP response?
			}
		}
	}
}

void	WebServer::_handleRequest( int connfd )
{
	HTTPrequest_t	req;
	HTTPreqStatus_t	status;
	pid_t 			child = -1;
	int				exitStat = 0;

	status = HTTPparser::parse(connfd, &req);
	if (status != FMT_OK)
		std::cout << "parse request error: " << HTTPparser::printStatus(status) << '\n';
	else
		std::cout << "request received\n";
	HTTPparser::printData(req);
	
	// tokenize request [at least implement GET POST DELETE]
	
	child = fork();
	if (child == -1)
		throw(ServerException({"fork failed"}));
	else if (child == 0)
	{
		// pipe setup (create a pipe and then dup one end to the client fd?)
		// execve stuff ...
		exit(1);
	}
	if (waitpid(child, &exitStat, 0) < 0)
		throw(ServerException({"error while terminating process"}));
	this->_dropConn(connfd);
	// else if (WIFEXITED(exitStat) == 0)
	// 	std::cout << "child process killed by signal (unexpected)\n";
	// else if (WEXITSTATUS(exitStat) == 1)
	// 	std::cout << "bad request format\n";
	// return (status);
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