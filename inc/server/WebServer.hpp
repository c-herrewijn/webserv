/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 18:19:29 by fra           #+#    #+#                 */
/*   Updated: 2024/01/16 16:42:09 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <unistd.h>           // execve, dup, dup2, pipe, fork, access
#include <cstring>           // strerror
#include <netdb.h>            // gai_strerror, getaddrinfo, freeaddrinfo
#include <cerrno>            // errno
#include <sys/socket.h>       // socketpair, htons, htonl, ntohs, ntohl, select
// #include <sys/epoll.h>     // epoll_create, epoll_ctl, epoll_wait
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
// #include <filesystem>           // filesystem::path, access()
#include <iostream>
#include <initializer_list>
#include <string>
#include <vector>
#include <set>
#include <map>
#include "HTTPparser.hpp"
#include "HTTPexecutor.hpp"
#define BACKLOG 	10				        // max pending connection queued up
#define MAX_TIMEOUT 60000               	// maximum timeout with poll()

class ServerException : std::exception
{
	public:
		ServerException( std::initializer_list<const char*> prompts) noexcept;
		virtual const char* what() const noexcept override {return (this->_msg.c_str());}
		virtual ~ServerException( void ) noexcept {}
	
	private:
		std::string _msg;
};

// NB: non-blocking setup sockets
// NB: non-blocking waitpid
// NB: chucked requests
class WebServer
{
	public:
		WebServer ( std::string const& servName) : _hostName(servName) {};
		~WebServer ( void ) noexcept;

		void			listenAt( const char*, const char* );
		void			loop( void );
		std::string		getAddress( const struct sockaddr_storage*) const noexcept ;

	private:
		std::string					_hostName;
		std::vector<struct pollfd>	_connfds;
		std::set<int>				_listeners;

		WebServer ( void ) {};
		WebServer ( WebServer const& ) noexcept;
		WebServer& operator=( WebServer const& ) noexcept;

		void		_dropConn( int socket = -1 ) noexcept;
		void		_addConn( int ) noexcept;
		void		_acceptConnection( int ) ;
		void		_handleRequest( int ) ;
		bool		_isListener( int ) const ;
		std::string	_readSocket( int ) const ;
		void		_writeSocket( int, std::string const& ) const ;
};