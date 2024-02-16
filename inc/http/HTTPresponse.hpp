/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPresponse.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:01:20 by fra           #+#    #+#                 */
/*   Updated: 2024/02/16 16:59:06 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <ctime>
#include <sys/types.h>        // send, recv
#include <sys/socket.h>       // send, recv

#include "HTTPstruct.hpp"
#define	HTTP_CGI_STR	std::string("CGI")

class HTTPresponse : public HTTPstruct
{
	public:
		HTTPresponse( void ) : HTTPstruct() {};
		virtual ~HTTPresponse( void ) override {};

		void		parseFromCode( int, std::string const&, std::string const& ) noexcept;
		void		parseFromCGI( std::string const& ) noexcept;
		void		parseBody( std::string const& ) noexcept override;
		void		writeContent( int socket=-1 ) ;
		std::string	toString( void ) const noexcept override;

		int					getStatusCode( void ) const noexcept;
		std::string const&	getStatusStr( void ) const noexcept;

	protected:
		int         _statusCode;
		std::string _statusStr;

		
		std::string	_mapStatusCode( int ) const ;
		void		_setHead( std::string const& ) override;
		// void		_setVersion( std::string const& ) override;
		// void		_setStatusCode( std::string const& );
		// void		_setStatusStr( std::string const& );

		std::string	_getDateTime( void ) const noexcept;
};