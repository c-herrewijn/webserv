/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 20:16:40 by fra           #+#    #+#                 */
/*   Updated: 2023/12/06 19:50:37 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "headers.hpp"

class ClientException : std::exception
{
	public:
		ClientException( std::initializer_list<const char*> prompts) noexcept;
		virtual const char* what() const noexcept override {return (this->_msg.c_str());}
		virtual ~ClientException( void ) noexcept {}
	
	private:
		std::string _msg;
};

class Client
{
	public:
		Client( void ) noexcept;
		~Client( void ) noexcept;

		void		connectTo( const char*, const char* );
		void		sendRequest( const char* ) const ;
		std::string	getAddress( const struct sockaddr_storage*) const noexcept ;

	private:
		int 						_sockfd = -1;
		const char					*_connectTo, *_port;
		struct sockaddr_storage 	_serverAddr;
		struct addrinfo 			_filter;

		Client( Client const& ) noexcept;
		Client&	operator=( Client const& ) noexcept;
};