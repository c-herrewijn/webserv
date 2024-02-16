/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 17:05:42 by faru          #+#    #+#                 */
/*   Updated: 2024/02/16 11:03:12 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HTTPstruct.hpp"
#include <sys/types.h>        // send, recv
#include <sys/socket.h>       // send, recv
#include <cstring>           // strerror
#include <limits>
#define HEADER_BUF_SIZE 	1024						// max size of HTTP header

typedef enum HTTPmethod_s
{
	HTTP_GET,
	HTTP_POST,
	HTTP_DELETE,
} HTTPmethod;

typedef struct HTTPurl_f
{
	std::string	scheme;
	std::string	host;
	int			port;
	std::string	path;
	dict		query;
	std::string queryRaw;
	std::string fragment;

} HTTPurl;

class HTTPrequest : public HTTPstruct
{
	public:
		HTTPrequest( void ) : HTTPstruct() {};
		virtual ~HTTPrequest( void ) override {};

		void		readHead( int socket=-1 );
		void		readRemainingBody( size_t ) ;
		void		parseBody( std::string const& strBody="" ) override;
		std::string	toString( void ) const noexcept override;

		HTTPmethod const& 	getMethod( void ) const noexcept;
		std::string	const& 	getPath( void ) const noexcept;
		std::string		 	getHost( void ) const noexcept;
		std::string	const& 	getBody( void ) const noexcept;
		std::string	const&	getQueryRaw( void ) const noexcept;

	protected:
		HTTPmethod	_method;
		HTTPurl		_url;
		std::string	_tmpBody;

		void	_setHead( std::string const& ) override;
		void	_setHeaders( std::string const& ) override;
		void	_setBody( std::string const& ) override;

		void	_setMethod( std::string const& );
		void	_setURL( std::string const& );
		void	_setVersion( std::string const& );

		void	_setScheme( std::string const& );
		void	_setHostPort( std::string const& );
		void	_setPath( std::string const& );
		void	_setQuery( std::string const& );
		void	_setFragment( std::string const& );

		void	_setChunkedBody( std::string const& );
};
