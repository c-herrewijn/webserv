/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: itopchu <itopchu@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/25 18:19:29 by fra               #+#    #+#             */
/*   Updated: 2023/12/12 16:36:12 by itopchu          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include "HTTPparser.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <vector>
#include <set>
#define BACKLOG 100				        // max pending connection queued up
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
		WebServer ( void );
		~WebServer ( void ) noexcept;

		struct addrinfo	getFilter( void ) const noexcept ;
		void			setFilter( struct addrinfo const& ) noexcept;
		void			listenAt( const char*, const char* );
		void			loop( void );
		std::string		getAddress( const struct sockaddr_storage*) const noexcept ;

	private:
		std::vector<struct pollfd>	_connfds;
		std::set<int>				_listeners;
		struct addrinfo 			_filter;

		WebServer ( WebServer const& ) noexcept;
		WebServer& operator=( WebServer const& ) noexcept;

		void	_dropConn( int socket = -1 ) noexcept;
		void	_addConn( int ) noexcept;
		void	_acceptConnection( int ) ;
		void	_handleRequest( int ) ;
		bool	_isListener( int ) const ;
};

#endif
