/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 18:19:29 by fra           #+#    #+#                 */
/*   Updated: 2024/02/23 17:52:59 by faru          ########   odam.nl         */
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
#include <fcntl.h>

#include "HTTPrequest.hpp"
#include "HTTPresponse.hpp"
// #include "RequestExecutor.hpp"
#include "Exceptions.hpp"
#include "ConfigServer.hpp"
#define BACKLOG 			10				        	// max pending connection queued up

enum fdType
{
    SERVER_SOCKET,
    CLIENT_CONNECTION,
    CGI_DATA_PIPE,
    CGI_RESPONSE_PIPE,
    STATIC_FILE
};

enum fdState
{
	WAITING_FOR_CONNECTION,			// SERVER_SOCKET (read)
	READ_REQ_HEADER,				// CLIENT_CONNECTION (read)
	READ_STATIC_FILE,				// STATIC_FILE (read)
	FORWARD_REQ_BODY_TO_CGI,		// CLIENT_CONNECTION (read), CGI_DATA_PIPE (write)
	READ_CGI_RESPONSE,				// CGI_RESPONSE_PIPE (read)
	WRITE_TO_CLIENT					// CLIENT_CONNECTION (write)
};

typedef struct PollItem
{
	int				fd;
	fdType          pollType;
    fdState         pollState;
    bool			actionHappened;
} t_PollItem;


// NB: non-blocking waitpid
// NB: in case of terminating error child process must be killed with signals
class WebServer
{
	public:
		WebServer ( std::vector<ConfigServer> const& );
		~WebServer ( void ) noexcept;

		void			startListen( void );
		void			loop( void );

		std::string		getAddress( const struct sockaddr_storage*) const noexcept ;
		ConfigServer const&	getHandler( std::string const& ) const ;
		ConfigServer const&	getDefaultServer( void ) const ;

	private:
		ConfigServer				 _defaultServer;
		std::vector<ConfigServer>	 _servers;
		std::vector<Listen>			 _listenAddress;
		std::vector<struct pollfd>	 _pollfds;
		std::unordered_map<int, t_PollItem>	 	_pollitems;
		std::unordered_map<int, HTTPrequest*> 	_requests;

		void			_listenTo( std::string const&, std::string const& );
		HTTPresponse	_handleRequest( int ) ;
		void			_addConn( int , fdType , fdState ) noexcept;
		void			_dropConn( int socket = -1 ) noexcept;

		void			handleNewConnections( PollItem& ); // keep - DONE
		void			readRequestHeaders( PollItem& ); // keep / rework
		void			readStaticFiles( PollItem& ); // keep / rework
		void			forwardRequestBodyToCGI( PollItem& ); // split into: 'readRequestBody()' and 'writeRequestBodyToCGI()'
		void			readCGIResponses( PollItem& ); // keep / rework
		void			writeToClients( PollItem& );
};
