/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 18:19:29 by fra           #+#    #+#                 */
/*   Updated: 2023/11/26 05:43:44 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "headers.hpp"

class ServerException : std::exception
{
	public:
		ServerException( std::string const& msg) noexcept : std::exception() , _msg(msg) {};
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
		void			start( void );
		void			printAddress( struct sockaddr_storage*, const char* preMsg=nullptr ) const noexcept ;

	private:
		int 			_sockfd, _backlog;
		const char*		_port;
		struct addrinfo _filter;
		struct sockaddr_storage	_host;

		const char*	_testPort(const char *) const ;
		void		_clearUsage(int, int, int);
		void		_interactWithClient( int );
		void		_parseHTTP( int ) const;
		Server ( Server const& ) noexcept;
		Server& operator=( Server const& ) noexcept;
};