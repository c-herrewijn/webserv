/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 17:56:25 by fra           #+#    #+#                 */
/*   Updated: 2023/11/26 03:05:42 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server ( void ) : _port(_testPort("80"))
{
	this->_readfd = -1;
	this->_writefd = -1;
	memset(&this->_filter, 0, sizeof(this->_filter));
	this->_filter.ai_flags = AI_PASSIVE;
	this->_filter.ai_family = AF_UNSPEC;
	this->_filter.ai_protocol = IPPROTO_TCP;
	this->_backlog = BACKLOG;
	memset(&this->_host, 0, sizeof(struct sockaddr));
	memset(&this->_client, 0, sizeof(struct sockaddr_storage));
}

Server::Server ( const char *port, struct addrinfo *filter) : _port(_testPort(port))
{
	this->_readfd = -1;
	this->_writefd = -1;
	if (filter == nullptr)
	{
		memset(&this->_filter, 0, sizeof(this->_filter));
		this->_filter.ai_flags = AI_PASSIVE;
		this->_filter.ai_family = AF_UNSPEC;
		this->_filter.ai_protocol = IPPROTO_TCP;
	}
	else
		this->_filter = *filter;
	this->_backlog = BACKLOG;
	memset(&this->_host, 0, sizeof(struct sockaddr));
	memset(&this->_client, 0, sizeof(struct sockaddr_storage));
}

Server::Server ( Server const& other ) noexcept : _port("")
{
	// ofc shallow copy of port and IP attributes would be problematic because
	// of the cuncurrency of two servers accessing the same ip:port, if fact it 
	// makes no sense to create copies of servers
	(void) other;
}

Server::~Server ( void ) noexcept
{
	if (this->_readfd != -1)
		close(this->_readfd);
	if (this->_writefd != -1)
		close(this->_writefd);
}

Server& Server::operator=( Server const& other ) noexcept
{
	// see copy constructor
	(void) other;
	return (*this);
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
	struct addrinfo *list, *tmp;

	if (getaddrinfo(NULL, this->_port, &this->_filter, &list) == -1)
		throw(ServerException("error: failed to get addresses"));
	for (tmp=list; tmp!=nullptr; tmp=tmp->ai_next)
	{
		this->_readfd = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
		if (this->_readfd == -1)
			continue;
		try
		{
			_clearUsage(SOL_SOCKET, SO_REUSEADDR, 1);
		}
		catch (ServerException const& e)
		{
			freeaddrinfo(list);
			throw(e);
		}
		if (bind(this->_readfd, tmp->ai_addr, tmp->ai_addrlen) == -1)
		{
			close(this->_readfd);
			continue;
		}
		break;
	}
	if (tmp == nullptr)
	{
		freeaddrinfo(list);
		throw(ServerException("error : no available IP host found"));
	}
	memmove(&this->_host, tmp->ai_addr, std::min(sizeof(struct sockaddr), sizeof(struct sockaddr_storage)));
	printAddress(&this->_host, "binded on: ");
	freeaddrinfo(list);
}

void	Server::start( void )
{
	unsigned int sizeAddr = sizeof(this->_client);
	if (listen(this->_readfd, this->_backlog) == -1)
		throw(ServerException("error : listening failed"));
	this->_writefd = accept(this->_readfd, (struct sockaddr *) &this->_client, &sizeAddr);
	if (this->_writefd == -1)
		throw(ServerException("error : failed connection with client"));
	printAddress(&this->_client,"connection established with client: ");
}

void	Server::interactWithClient( void )
{
	char buf[MAX_INPUT];
	memset(buf, 0, MAX_INPUT);
	send(this->_writefd, "Halo!\n", 6, 0);
	recv(this->_writefd, buf, MAX_INPUT, 0);
	std::cout << buf;
}

void	Server::printAddress( struct sockaddr_storage *addr, const char* preMsg ) const noexcept
{
	if (preMsg != nullptr)
		std::cout << preMsg;
	if (addr->ss_family == AF_INET)
	{
		char ipv4[INET_ADDRSTRLEN];
		struct sockaddr_in *addr_v4 = (struct sockaddr_in*) addr;
		inet_ntop(addr_v4->sin_family, &(addr_v4->sin_addr), ipv4, sizeof(ipv4));
		std::cout << ipv4 << ":" << ntohs(addr_v4->sin_port) << "\n";
	}
	else if (addr->ss_family == AF_INET6)
	{
		char ipv6[INET6_ADDRSTRLEN];
		struct sockaddr_in6 *addr_v6 = (struct sockaddr_in6*) addr;
		inet_ntop(addr_v6->sin6_family, &(addr_v6->sin6_addr), ipv6, sizeof(ipv6));
		std::cout << ipv6 << ":" << ntohs(addr_v6->sin6_port) << "\n";
	}
}

const char*	Server::_testPort(const char *port) const
{
	if (socket(PF_INET, SOCK_STREAM, 0) == -1)
		throw(ServerException("error: port not available"));
	return (port);
}

void	Server::_clearUsage(int level, int optname, int value)
{
	if (setsockopt(this->_readfd, level, optname, &value, sizeof(value)) == -1)
		throw(ServerException("error: socket update failed"));
}
