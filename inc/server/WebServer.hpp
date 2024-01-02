/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 18:19:29 by fra           #+#    #+#                 */
/*   Updated: 2023/12/31 14:23:54 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <unistd.h>           // execve, dup, dup2, pipe, fork, access
#include <cstring>           // strerror
#include <netdb.h>            // gai_strerror, getaddrinfo, freeaddrinfo
#include <cerrno>            // errno
#include <sys/socket.h>       // socketpair, htons, htonl, ntohs, ntohl, select
#include <sys/epoll.h>     // epoll_create, epoll_ctl, epoll_wait
#include <sys/poll.h>     // poll
#include <netinet/in.h>       // socket, accept, listen, bind, connect
#include <arpa/inet.h>        // htons, htonl, ntohs, ntohl
#include <sys/types.h>        // send, recv
#include <sys/socket.h>       // send, recv
#include <sys/types.h>        // chdir
#include <sys/stat.h>         // stat
#include <sys/wait.h>         // waitpid
#include <fcntl.h>            // open
#include <dirent.h>           // opendir, readdir, closedir
#include <signal.h>           // kill, signal
#include <filesystem>           // filesystem::path, 
#include <iostream>
#include <initializer_list>
#include <string>
#include <vector>
#include <set>
#include <map>
#include "HTTPparser.hpp"
#define BACKLOG 10				        // max pending connection queued up
#define MAX_TIMEOUT 60000               // maximum timeout with poll()

class ServerException : std::exception
{
	public:
		ServerException( std::initializer_list<const char*> prompts) noexcept;
		virtual const char* what() const noexcept override {return (this->_msg.c_str());}
		virtual ~ServerException( void ) noexcept {}
	
	private:
		std::string _msg;
};

class WebServer
{
	public:
		WebServer ( void ) {};
		~WebServer ( void ) noexcept;

		void			listenAt( const char*, const char* );
		void			loop( void );
		std::string		getAddress( const struct sockaddr_storage*) const noexcept ;

	private:
		std::vector<struct pollfd>	_connfds;
		std::set<int>				_listeners;

		WebServer ( WebServer const& ) noexcept;
		WebServer& operator=( WebServer const& ) noexcept;

		void		_dropConn( int socket = -1 ) noexcept;
		void		_addConn( int ) noexcept;
		void		_acceptConnection( int ) ;
		void		_handleRequest( int ) ;
		bool		_isListener( int ) const ;
		std::string	_readSocket( int ) const ;
};