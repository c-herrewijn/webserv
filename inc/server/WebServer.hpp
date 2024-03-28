/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 18:19:29 by fra           #+#    #+#                 */
/*   Updated: 2024/03/27 21:07:41 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <netdb.h>          // gai_strerror, getaddrinfo, freeaddrinfo
#include <cerrno>           // errno
#include <sys/poll.h>     	// poll
#include <netinet/in.h>     // socket, accept, listen, bind, connect
#include <arpa/inet.h>      // htons, htonl, ntohs, ntohl
#include <signal.h>         // kill, signal
#include <iostream>			// I/O streaming
#include <fstream>			// file streaming
#include <string>			// std::string class
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "HTTPrequest.hpp"
#include "HTTPresponse.hpp"
#include "Exceptions.hpp"
#include "ConfigServer.hpp"
#include "CGI.hpp"

#define BACKLOG 			10		// max pending connection queued up
#define SERVER_DEF_PAGES	std::filesystem::path("var/www/errors")

using namespace std::chrono;

enum fdType
{
    LISTENER,
    CLIENT_CONNECTION,
    CGI_REQUEST_PIPE_WRITE_END,
    CGI_RESPONSE_PIPE_READ_END,
    STATIC_FILE
};

enum fdState
{
	WAITING_FOR_CONNECTION,			// LISTENER (read)
	READ_REQ_HEADER,				// CLIENT_CONNECTION (read)
	READ_STATIC_FILE,				// STATIC_FILE (read)
	READ_REQ_BODY,					// CLIENT_CONNECTION (read)
	WAIT_FOR_CGI,					// CLIENT_CONNECTION (no action)
	READ_CGI_RESPONSE,				// CGI_RESPONSE_PIPE (read)
	WRITE_TO_CLIENT,				// CLIENT_CONNECTION (write)
	WRITE_TO_CGI					// CGI_REQUEST_PIPE (write)
};

typedef struct PollItem
{
	int		fd;
	fdType  pollType;
    fdState pollState;
} t_PollItem;

class WebServer
{
	public:
		WebServer ( std::vector<ConfigServer> const& );
		~WebServer ( void ) noexcept;

		void	run( void );

	private:
		ConfigServer				 			_defaultServer;
		std::vector<ConfigServer>	 			_servers;
		std::vector<struct pollfd>	 			_pollfds;
		std::unordered_map<int, t_PollItem*>	_pollitems;
		std::unordered_map<int, HTTPrequest*> 	_requests;
		std::unordered_map<int, HTTPresponse*> 	_responses;
		std::unordered_map<int, CGI*> 			_cgi;	// NOTE: the key is the client socket fd, not any of the cgi-pipes
		std::vector<int>						_emptyConns;

		void		_listenTo( std::string const&, std::string const& );
		void		_readData( int );
		void		_writeData( int );
		void		_addConn( int , fdType , fdState );
		void		_dropConn( int ) noexcept;
		void		_dropStructs( int ) noexcept;
		void		_clearEmptyConns( void ) noexcept;
		std::string	_getAddress( const struct sockaddr_storage*) const noexcept ;
		int			_getSocketFromFd( int );
		std::vector<ConfigServer const&>	_getServersFromIP( std::string const& ) const noexcept;

		void	handleNewConnections( int ); // keep - DONE
		void	readRequestHeaders( int ); // keep / rework
		void	_addSecondaryConnections( int );
		void	readStaticFiles( int ); // keep / rework
		void	readRequestBody( int );
		void	readCGIResponses( int ); // keep / rework
		void	writeToCGI( int item );
		void	writeToClients( int );
		void	redirectToErrorPage( int, int ) noexcept;
};
