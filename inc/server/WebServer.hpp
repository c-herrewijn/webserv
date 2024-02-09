/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 18:19:29 by fra           #+#    #+#                 */
/*   Updated: 2024/02/09 18:53:17 by faru          ########   odam.nl         */
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
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <filesystem>
#include <fcntl.h>
#include "HTTPrequest.hpp"
#include "HTTPresponse.hpp"
#include "Exception.hpp"
#include "Server.hpp"
#define HEADER_MAX_SIZE 	1024						// max size of HTTP header
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

		void			loop( void );
		void			startListen( void );
		std::string		getAddress( const struct sockaddr_storage*) const noexcept ;
		Server const&	getHandler( std::string const& ) const ;
		Server const&	getDefaultServer( void ) const ;

	private:
		std::vector<Server>			_servers;
		std::vector<Listen>			_listenAddress;
		std::vector<struct pollfd>	_connfds;
		std::set<int>				_listeners;

		void	_listenTo( std::string const&, std::string const& );
		void	_dropConn( int socket = -1 ) noexcept;
		void	_addConn( int ) noexcept;
		void	_acceptConnection( int ) ;
		int		_handleRequest( int ) ;		// NB: is the status (i.e. returned value) necessary?
		bool	_isListener( int ) const ;
		int		_readHead( int , std::string&, std::string& ) const;
		void	_writeSocket( int, std::string const& ) const ;
		// void			_waitForChildren( void) ;
};