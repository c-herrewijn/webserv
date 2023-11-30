/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 17:56:25 by fra           #+#    #+#                 */
/*   Updated: 2023/11/30 01:38:08 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

ServerException::ServerException( std::initializer_list<const char*> prompts) noexcept 
	: std::exception()
{
	this->_msg = " Webserver exception: ";
	for (const char *prompt : prompts)
		this->_msg += std::string(prompt) + " ";
}


Server::Server ( void ) : _port("80")
{
	this->_sockfd = -1;
	this->_connfd = -1;
	memset(&this->_filter, 0, sizeof(struct addrinfo));
	this->_filter.ai_flags = AI_PASSIVE;
	this->_filter.ai_family = AF_UNSPEC;
	this->_filter.ai_protocol = IPPROTO_TCP;
	this->_backlog = BACKLOG;
	memset(&this->_host, 0, sizeof(struct sockaddr));
}

Server::Server ( const char *port, struct addrinfo *filter) : _port(port)
{
	this->_sockfd = -1;
	this->_connfd = -1;
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
	if (this->_sockfd != -1)
		close(this->_sockfd);
	if (this->_connfd != -1)
		close(this->_connfd);
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
		this->_sockfd = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
		if (this->_sockfd == -1)
			continue;
		if (setsockopt(this->_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0)
			std::cout << "failed to update socket, trying to bind anyway... \n";
		if (bind(this->_sockfd, tmp->ai_addr, tmp->ai_addrlen) == 0)
			break;
		close(this->_sockfd);
	}
	if (tmp == nullptr)
	{
		freeaddrinfo(list);
		throw(ServerException({"no available IP host found for port", this->_port}));
	}
	memmove(&this->_host, tmp->ai_addr, std::min(sizeof(struct sockaddr), sizeof(struct sockaddr_storage)));
	freeaddrinfo(list);
	std::cout << "binded on: " << this->getAddress(&this->_host) << "\n";
	if (listen(this->_sockfd, this->_backlog) != 0)
		throw(ServerException({"listening on", this->getAddress(&this->_host).c_str()}));
}

void	Server::handleSequentialConn( void )
{
	struct sockaddr_storage 	client;
	unsigned int 				sizeAddr = sizeof(client);
	int							connfd = -1;
	while (true)
	{
		connfd = accept(this->_sockfd, (struct sockaddr *) &client, &sizeAddr);
		if (connfd == -1)
		{
			std::cout << "failed connection with: " << this->getAddress(&client) << '\n';
			continue;
		}
		std::cout << "connected to " << this->getAddress(&client) << '\n';
		try
		{
			this->parseRequest(connfd);
			this->handleRequest();
		}
		catch(ServerException const& err)
		{
			close(connfd);
			throw(err);	
		}
		close(connfd);
	}
}

void	Server::parseRequest( int connfd )
{
	try
	{
		this->_parser.parse(connfd);
	}
	catch(ServerException const& err)
	{
		std::cerr << err.what() << '\n';
		return ;
	}
	this->_parser.printData();
}

void	Server::handleRequest( void )
{
	pid_t 	child = -1;
	int		exitStat = 0;

	child = fork();
	if (child == -1)
		throw(ServerException({"fork failed"}));
	else if (child == 0)
	{
		// do stuff ...
	}
	if (waitpid(child, &exitStat, 0) < 0)
		throw(ServerException({"error while terminating process"}));
	else if (WIFEXITED(exitStat) == 0)
		std::cout << "child process killed by signal (unexpected)\n";
	else if (WEXITSTATUS(exitStat) == 1)
		std::cout << "bad request format\n";
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