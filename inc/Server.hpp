/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 18:19:29 by fra           #+#    #+#                 */
/*   Updated: 2023/12/08 03:57:46 by fra           ########   odam.nl         */
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
		~Server ( void ) noexcept;

		struct addrinfo	getFilter( void ) const noexcept ;
		void			setFilter( struct addrinfo const& ) noexcept;
		void			listenAt( const char*, const char* );
		void			loop( void );
		std::string		getAddress( const struct sockaddr_storage*) const noexcept ;

	private:
		std::vector<struct pollfd>	_connfds;
		std::set<int>				_listeners;
		struct addrinfo 			_filter;

		Server ( Server const& ) noexcept;
		Server& operator=( Server const& ) noexcept;

		void	_dropConn( int socket = -1 ) noexcept;
		void	_addConn( int ) noexcept;
		void	_acceptConnection( int ) ;
		void	_handleRequest( int ) ;
		bool	_isListener( int ) const ;
};