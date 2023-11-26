/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 20:22:27 by fra           #+#    #+#                 */
/*   Updated: 2023/11/26 21:13:54 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client( void ) : _port("80") , _connectTo("localhost")
{
	memset(&this->_filter, 0, sizeof(this->_filter));
	this->_filter.ai_flags = AI_PASSIVE;
	this->_filter.ai_family = AF_UNSPEC;
	this->_filter.ai_socktype = SOCK_STREAM;
}

Client::Client( const char* port, const char* host ) : _port(port) , _connectTo(host)
{
	memset(&this->_filter, 0, sizeof(this->_filter));
	this->_filter.ai_flags = AI_PASSIVE;
	this->_filter.ai_family = AF_UNSPEC;
	this->_filter.ai_socktype = SOCK_STREAM;
}

Client::Client( Client const& other )
{
	(void) other;
}

Client::~Client( void )
{
	if (this->_sockfd != -1)
		close(this->_sockfd);
}

Client&	Client::operator=( Client const& other )
{
	(void) other;
	return (*this);
}

void	Client::findServer( void ) 
{
	struct addrinfo *list, *tmp;

	if (getaddrinfo(this->_connectTo, this->_port, &this->_filter, &list) == -1)
		throw(ClientException("error: failed to get addresses"));
	for (tmp=list; tmp!=nullptr; tmp=tmp->ai_next)
	{
		this->_sockfd = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
		if (this->_sockfd == -1)
			continue;
		if (connect(this->_sockfd, tmp->ai_addr, tmp->ai_addrlen) != -1)
			break;
		// memmove(&this->_serverAddr, tmp->ai_addr, std::min(sizeof(struct sockaddr), sizeof(struct sockaddr_storage)));
		// printAddress(&this->_serverAddr, "found: ");
		close(this->_sockfd);
	}
	if (tmp == nullptr)
	{
		freeaddrinfo(list);
		throw(ClientException("error : no available connection"));
	}
	
	memmove(&this->_serverAddr, tmp->ai_addr, std::min(sizeof(struct sockaddr), sizeof(struct sockaddr_storage)));
	printAddress(&this->_serverAddr, "binded on: ");
	freeaddrinfo(list);
}

void	Client::sendRequest( const char *request ) const 
{
	ssize_t	sendBytes;

	sendBytes = send(this->_sockfd, request, strlen(request), 0);
	if (sendBytes < (ssize_t) strlen(request))
		throw(ClientException("error: failed to send the message to server"));
	close(this->_sockfd);
}

void	Client::printAddress( struct sockaddr_storage *addr, const char* preMsg ) const noexcept
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