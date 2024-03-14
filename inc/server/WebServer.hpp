/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 18:19:29 by fra           #+#    #+#                 */
/*   Updated: 2024/03/14 21:30:27 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <unistd.h>           // execve, dup, dup2, pipe, fork, access
#include <cstring>           // strerror
#include <netdb.h>            // gai_strerror, getaddrinfo, freeaddrinfo
#include <cerrno>            // errno
#include <sys/socket.h>       // socketpair, htons, htonl, ntohs, ntohl, select
#include <sys/poll.h>     // poll
#include <netinet/in.h>       // socket, accept, listen, bind, connect
#include <arpa/inet.h>        // htons, htonl, ntohs, ntohl
// #include <sys/types.h>        // send, recv
// #include <sys/socket.h>       // send, recv
#include <sys/wait.h>         // waitpid
// #include <fcntl.h>            // open
// #include <sys/types.h>        // chdir
// #include <sys/stat.h>         // stat
// #include <dirent.h>           // opendir, readdir, closedir
#include <signal.h>           // kill, signal
// #include <limits>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <stack>
#include <unordered_map>
#include <algorithm>
#include <filesystem>

#include "HTTPrequest.hpp"
#include "HTTPresponse.hpp"
#include "Exceptions.hpp"
#include "ConfigServer.hpp"
#include "CGI.hpp"

#define BACKLOG 				10		// max pending connection queued up
#define HTML_ERROR_FOLDER		std::filesystem::path("var/www/errors")

using namespace std::chrono;

enum fdType
{
    LISTENER,
    CLIENT_CONNECTION,
    CGI_REQUEST_PIPE,
    CGI_RESPONSE_PIPE,
    STATIC_FILE
};

enum fdState
{
	WAITING_FOR_CONNECTION,			// LISTENER (read)
	READ_REQ_HEADER,				// CLIENT_CONNECTION (read)
	READ_STATIC_FILE,				// STATIC_FILE (read)
	READ_REQ_BODY,					// CLIENT_CONNECTION (read)
	READ_CGI_RESPONSE,				// CGI_RESPONSE_PIPE (read)
	WRITE_TO_CLIENT,				// CLIENT_CONNECTION (write)
	WRITE_TO_CGI					// CGI_REQUEST_PIPE (write)
};


typedef struct PollItem
{
	int							fd;
	fdType          			pollType;
    fdState         			pollState;
} t_PollItem;

// NB: in case of terminating error child process must be killed with signals
class WebServer
{
	public:
		WebServer ( std::vector<ConfigServer> const& );
		~WebServer ( void ) noexcept;

		void			startListen( void );
		void			loop( void );

	private:
		ConfigServer				 _defaultServer;
		std::vector<ConfigServer>	 _servers;
		std::vector<Listen>			 _listenAddress;
		std::vector<struct pollfd>	 _pollfds;
		std::unordered_map<int, t_PollItem*>	 _pollitems;
		std::unordered_map<int, HTTPrequest*> 	_requests;
		std::unordered_map<int, HTTPresponse*> 	_responses;
		std::unordered_map<int, CGI*> 			_cgi;	// NOTE: the key is the client socket fd, not any of the cgi-pipes
		std::vector<int>						_emptyConns;

		void			_listenTo( std::string const&, std::string const& );
		void			_readData( int );
		void			_writeData( int );
		void			_addConn( int , fdType , fdState );
		void			_dropConn( int ) noexcept;
		void			_dropStructs( int ) noexcept;
		void			_clearEmptyConns( void ) noexcept;
		std::string			_getAddress( const struct sockaddr_storage*) const noexcept ;
		ConfigServer const&	_getHandler( std::string const& ) const noexcept;
		ConfigServer const&	_getDefaultHandler( void ) const noexcept;
		int				_getSocketFromFd( int );
		t_path			_getHTMLerrorPage( int, t_string_map const& ) const;
		void			handleNewConnections( int ); // keep - DONE
		void			readRequestHeaders( int ); // keep / rework
		void			readStaticFiles( int ); // keep / rework
		void			readRequestBody( int );
		void			readCGIResponses( int ); // keep / rework
		void			writeToCGI( int item );
		void			writeToClients( int );
		void			redirectToErrorPage( int, int ) noexcept;

		void			_startCGI( int );
		void			_addStaticFileFd( std::string const&, int );
		void			_parseResponse( int );
};
