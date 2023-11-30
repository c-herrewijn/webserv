/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 18:19:29 by fra           #+#    #+#                 */
/*   Updated: 2023/11/30 01:38:24 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "headers.hpp"
#include "HTTPparser.hpp"

class ServerException : std::exception
{
	public:
		ServerException( std::initializer_list<const char*> prompts) noexcept;
		virtual const char* what() const noexcept override {return (this->_msg.c_str());}
		virtual ~ServerException( void ) noexcept {}
	
	private:
		std::string _msg;
};

class Server
{
	public:
		Server ( void );
		Server ( const char *, struct addrinfo *filter=nullptr);
		~Server ( void ) noexcept;

		const char		*getPort( void ) const noexcept ;
		struct addrinfo	getFilter( void ) const noexcept ;
		void			setFilter( struct addrinfo const& ) noexcept;
		void			bindPort( void );
		void			handleSequentialConn( void );
		void			parseRequest( int ) ;
		void			handleRequest( void ) ;
		std::string		getAddress( const struct sockaddr_storage*) const noexcept ;

	private:
		int 			_sockfd, _connfd, _backlog;
		const char*		_port;
		struct addrinfo _filter;
		struct sockaddr_storage	_host;
		HTTPparser		_parser;

		Server ( Server const& ) noexcept;
		Server& operator=( Server const& ) noexcept;
};