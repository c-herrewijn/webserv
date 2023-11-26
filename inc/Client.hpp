/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 20:16:40 by fra           #+#    #+#                 */
/*   Updated: 2023/11/26 21:02:06 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "headers.hpp"

class ClientException : std::exception
{
	public:
		ClientException( std::string const& msg) noexcept : std::exception() , _msg(msg) {};
		virtual const char* what() const noexcept override {return (this->_msg.c_str());}
		virtual ~ClientException( void ) noexcept {}
	
	private:
		std::string _msg;
};

class Client
{
	public:
		Client( void );
		Client( const char*, const char* );
		~Client( void );

		void	findServer( void );
		void	sendRequest( const char* ) const ;
		void	printAddress( struct sockaddr_storage*, const char* preMsg=nullptr ) const noexcept ;

	private:
		int 						_sockfd = -1;
		const char					*_port, *_connectTo;
		struct sockaddr_storage 	_serverAddr;
		struct addrinfo 			_filter;

		Client( Client const& );
		Client&	operator=( Client const& );
};