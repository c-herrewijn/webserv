/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 17:56:25 by fra           #+#    #+#                 */
/*   Updated: 2023/12/06 23:44:18 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

ServerException::ServerException( std::initializer_list<const char*> prompts) noexcept 
	: std::exception()
{
	this->_msg = "Webserver exception: ";
	for (const char *prompt : prompts)
		this->_msg += std::string(prompt) + " ";
}


Server::Server ( void ) : _port("80")
{
	this->_listener = -1;
	memset(&this->_filter, 0, sizeof(struct addrinfo));
	this->_filter.ai_flags = AI_PASSIVE;
	this->_filter.ai_family = AF_UNSPEC;
	this->_filter.ai_protocol = IPPROTO_TCP;
	this->_backlog = BACKLOG;
	memset(&this->_host, 0, sizeof(struct sockaddr));
}

Server::Server ( const char *port, struct addrinfo *filter) : _port(port)
{
	this->_listener = -1;
	if (filter == nullptr)
	{
		memset(&this->_filter, 0, sizeof(struct addrinfo));
		this->_filter.ai_flags = AI_PASSIVE;
		this->_filter.ai_family = AF_UNSPEC;
		this->_filter.ai_protocol = IPPROTO_TCP;
	}
	else
		this->_filter = *filter;
	this->_backlog = BACKLOG;
	memset(&this->_host, 0, sizeof(struct sockaddr));
}

Server::~Server ( void ) noexcept
{
	while(this->_connfds.empty() == false)
		this->_dropConn(0);
}

const char*	Server::getPort( void ) const noexcept
{
	return (this->_port);
}

struct addrinfo	Server::getFilter( void ) const noexcept
{
	return(this->_filter);
}

void	Server::setFilter( struct addrinfo const& newFilter ) noexcept
{
	this->_filter = newFilter;
}

void	Server::bindPort( void )
{
	struct addrinfo *tmp, *list;
	int yes = 1;

	if (getaddrinfo(NULL, this->_port, &this->_filter, &list) != 0)
		throw(ServerException({"failed to get addresses for port", this->_port}));
	for (tmp=list; tmp!=nullptr; tmp=tmp->ai_next)
	{
		this->_listener = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
		if (this->_listener == -1)
			continue;
		if (setsockopt(this->_listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0)
			std::cout << "failed to update socket, trying to bind anyway... \n";
		if (bind(this->_listener, tmp->ai_addr, tmp->ai_addrlen) == 0)
			break;
		close(this->_listener);
	}
	if (tmp == nullptr)
	{
		freeaddrinfo(list);
		throw(ServerException({"no available IP host found for port", this->_port}));
	}
	memmove(&this->_host, tmp->ai_addr, std::min(sizeof(struct sockaddr), sizeof(struct sockaddr_storage)));
	freeaddrinfo(list);
	std::cout << "binded on: " << this->getAddress(&this->_host) << "\n";
	if (listen(this->_listener, this->_backlog) != 0)
		throw(ServerException({"listening on", this->getAddress(&this->_host).c_str()}));
	this->_addConn(this->_listener);
}

int		Server::_acceptConnection( int listener )
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
	return (connfd);
}

void	Server::handleMultipleConn( void )
{
	int	nConn;

	while (true)
	{
		nConn = poll(this->_connfds.data(), this->_connfds.size(), 60000);
		if (nConn == -1)
			throw(ServerException({"poll failure"}));
		else if (nConn == 0)
			break;
		for (size_t i=0; i<this->_connfds.size(); i++)
		{
			if (this->_connfds[i].revents & POLLIN)
			{
				if (this->_connfds[i].fd == this->_listener) // new connection
					this->_acceptConnection(this->_listener);
				else			// new message
				{
					this->_handleRequest(this->_connfds[i].fd);
					this->_dropConn(i--);
				}
			}
		}
	}
}

void	Server::_handleRequest( int connfd ) const
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

	child = fork();
	if (child == -1)
		throw(ServerException({"fork failed"}));
	else if (child == 0)
	{
		sleep(1);
		exit(0);
		// pipe setup (create a pipe and then dup one end to the client fd?)
		// execve stuff ...
	}
	if (waitpid(child, &exitStat, 0) < 0)
		throw(ServerException({"error while terminating process"}));
	// else if (WIFEXITED(exitStat) == 0)
	// 	std::cout << "child process killed by signal (unexpected)\n";
	// else if (WEXITSTATUS(exitStat) == 1)
	// 	std::cout << "bad request format\n";
	HTTPparser::printData(req);
	// return (status);
}

std::string	Server::getAddress( const struct sockaddr_storage *addr ) const noexcept
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

Server::Server ( Server const& other ) noexcept : _port("")
{
	// ofc shallow copy of port and IP attributes would be problematic because
	// of the cuncurrency of two servers accessing the same ip:port, if fact it 
	// makes no sense to create copies of servers
	(void) other;
}

Server& Server::operator=( Server const& other ) noexcept
{
	// see copy constructor
	(void) other;
	return (*this);
}

void	Server::_dropConn(size_t pos) noexcept
{
	shutdown(this->_connfds[pos].fd, SHUT_RDWR);
	close(this->_connfds[pos].fd);
	this->_connfds.erase(this->_connfds.begin() + pos);
}

void	Server::_addConn( int newSocket ) noexcept
{
	struct pollfd	newfd;

	if (newSocket != -1)
	{
		newfd.fd = newSocket;
		newfd.events = POLLIN;
		newfd.revents = 0;
		this->_connfds.push_back(newfd);
	}
}