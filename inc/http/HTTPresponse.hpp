/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPresponse.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:01:20 by fra           #+#    #+#                 */
/*   Updated: 2024/02/18 03:33:29 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <ctime>
#include <sys/types.h>        // send, recv
#include <sys/socket.h>       // send, recv

#include "HTTPstruct.hpp"

#define STD_CONTENT_TYPE "text/html; charset=utf-8"

class HTTPresponse : public HTTPstruct
{
	public:
		HTTPresponse( void ) : HTTPstruct() , _statusCode(200) {};
		virtual ~HTTPresponse( void ) override {};

		void		parseFromStatic( int, std::string const&, std::string const& ) noexcept;
		void		parseFromCGI( int, std::string const& ) noexcept;
		void		writeContent( int socket=-1 ) ;
		std::string	toString( void ) const noexcept override;

		int					getStatusCode( void ) const noexcept;
		std::string const&	getStatusStr( void ) const noexcept;

	protected:
		int         _statusCode;

		void		_setHead( std::string const& ) override;
		void		_setBody( std::string const& ) override;
		std::string	_mapStatusCode( int ) const ;
		std::string	_getDateTime( void ) const noexcept;
};