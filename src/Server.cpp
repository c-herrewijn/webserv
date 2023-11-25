/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 17:56:25 by fra           #+#    #+#                 */
/*   Updated: 2023/11/25 21:41:17 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void getStrIP( struct sockaddr_storage *addr )
{
	if (addr->ss_family == AF_INET)
	{
		char ipv4[INET_ADDRSTRLEN];
		// ipv4[INET_ADDRSTRLEN] = '\0';
		struct sockaddr_in *addr_v4 = (struct sockaddr_in*) addr;
		// std::cout << "size v4: " << addr->ai_addrlen << "\n";
		inet_ntop(addr_v4->sin_family, &(addr_v4->sin_addr), ipv4, sizeof(ipv4));
		// write (1, ipv4, INET_ADDRSTRLEN);
		std::cout << ipv4 << "\n";
		// return (ipv4);
	}
	else if (addr->ss_family == AF_INET6)
	{
		char ipv6[INET6_ADDRSTRLEN];
		// ipv6[INET6_ADDRSTRLEN] = '\0';
		struct sockaddr_in6 *addr_v6 = (struct sockaddr_in6*) addr;
		// std::cout << "size v6: " << addr->ai_addrlen << "\n";
		inet_ntop(addr_v6->sin6_family, &(addr_v6->sin6_addr), ipv6, sizeof(ipv6));
		// write (1, ipv6, INET6_ADDRSTRLEN);
		std::cout << ipv6 << "\n";
		// return (ipv6);
	}
	// else
	// 	return ("porca madonna");        // throw smt

}

int createSocket( void )
{
	int sockfd, connfd;
	int yes = 1;
	struct addrinfo hints, *list, *servAddr;
	struct sockaddr_storage tmp;
	struct sockaddr_in *cliAddr;
	struct sockaddr_in6 *cliAddr6;
	unsigned int tmpSize = sizeof(tmp);

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_protocol = IPPROTO_TCP;
	if (getaddrinfo(NULL, PORT, &hints, &list) == -1)
		return (-1);
	for (servAddr=list; servAddr!=nullptr; servAddr=servAddr->ai_next)
	{
		// std::cout << "found address: ";
		// getStrIP((sockaddr_storage *) servAddr->ai_addr);
		sockfd = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
		if (sockfd == -1)
			continue;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		{
			close(sockfd);
			freeaddrinfo(list);
			return (-1);
		}
		if (bind(sockfd, servAddr->ai_addr, servAddr->ai_addrlen) == -1)
		{
			close(sockfd);
			continue;
		}
		else
			break;
	}
	if (servAddr == nullptr)
	{
		freeaddrinfo(list);
		return (-1);
	}
	// memset(&tmp, 0, sizeof(tmp));
	// tmp = (struct sockaddr_storage *) servAddr->ai_addr;
	// std::cout << "found available ip for connection: " << getStrIP((struct sockaddr_storage *) servAddr->ai_addr) << "\n";
	std::cout << "found address: ";
	getStrIP((sockaddr_storage *) servAddr->ai_addr);
	freeaddrinfo(list);
	if (listen(sockfd, BACKLOG) == -1)
	{
		close(sockfd);
		return (-1);
	}
	// memset(tmp, 0, sizeof(*tmp));
	connfd = accept(sockfd, (struct sockaddr *) &tmp, &tmpSize);
	if (connfd == -1)
	{
		close(sockfd);
		return (-1);
	}
	std::cout << "connection to client: ";
	getStrIP(&tmp);
	if (tmp.ss_family == AF_INET)
		cliAddr = (struct sockaddr_in *) &tmp;
	else if (tmp.ss_family == AF_INET6)
		cliAddr6 = (struct sockaddr_in6 *) &tmp;
	(void) cliAddr6;
	(void) cliAddr;
	close(connfd);
	close(sockfd);
	return (1);
}
