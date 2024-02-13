/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 18:19:29 by fra           #+#    #+#                 */
/*   Updated: 2024/02/13 17:23:33 by faru          ########   odam.nl         */
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
#include <sys/types.h>        // send, recv
#include <sys/socket.h>       // send, recv
#include <sys/wait.h>         // waitpid
// #include <fcntl.h>            // open
// #include <sys/types.h>        // chdir
// #include <sys/stat.h>         // stat
// #include <dirent.h>           // opendir, readdir, closedir
#include <signal.h>           // kill, signal
#include <limits>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <stack>
#include <algorithm>
#include <filesystem>
#include <fcntl.h>

#include "HTTPrequest.hpp"
#include "HTTPresponse.hpp"
#include "Executor.hpp"
#include "Exceptions.hpp"
#include "Server.hpp"
#define HEADER_BUF_SIZE 	1024						// max size of HTTP header
#define BACKLOG 			10				        	// max pending connection queued up
#define MAX_TIMEOUT 		60000               		// maximum timeout with poll()

// NB: non-blocking setup sockets
// NB: non-blocking waitpid
// NB: in case of terminating error child process must be killed with signals
class WebServer
{
	public:
		WebServer ( std::vector<Server> const& );
		~WebServer ( void ) noexcept;

		void			startListen( void );
		void			loop( void );

		std::string		getAddress( const struct sockaddr_storage*) const noexcept ;
		Server const&	getHandler( std::string const& ) const ;
		Server const&	getDefaultServer( void ) const ;

	private:
		Server						_defaultServer;
		std::vector<Server>			_servers;
		std::vector<Listen>			_listenAddress;
		std::vector<struct pollfd>	_connfds;
		std::set<int>				_listeners;

		void			_listenTo( std::string const&, std::string const& );
		// bool			_handleEvent(struct pollfd const&);
		void			_acceptConnection( int ) ;
		HTTPresponse	_handleRequest( int ) const ;
		std::string		_readHead( int ) const ;
		std::string		_readRemainingBody( int, size_t, size_t ) const ;
		void			_writeResponse( HTTPresponse const& ) const ;
		bool			_isListener( int ) const ;
		void			_addConn( int ) noexcept;
		void			_dropConn( int socket = -1 ) noexcept;
};