/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 18:19:29 by fra           #+#    #+#                 */
/*   Updated: 2023/12/30 01:46:32 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "global.hpp"
#include "HTTPparser.hpp"
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

// three ways to handle a connections:
// 1. short lived	--> new conn, req, resp, close conn, repeat
// 2. persistent	--> new conn, req, resp, (not mandatory) close conn, repeat
// 3. pipelining	--> sending several connections without waiting for a response
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

		void		_dropConn( int socket = -1 ) noexcept;
		void		_addConn( int ) noexcept;
		void		_acceptConnection( int ) ;
		void		_handleRequest( int ) ;
		bool		_isListener( int ) const ;
		std::string	_readSocket( int ) const ;
};