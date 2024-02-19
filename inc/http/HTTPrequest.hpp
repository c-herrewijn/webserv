/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 17:05:42 by faru          #+#    #+#                 */
/*   Updated: 2024/02/19 18:53:15 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HTTPstruct.hpp"
#include <sys/types.h>        // send, recv
#include <sys/socket.h>       // send, recv
#include <cstring>           // strerror
#include <limits>
#include <filesystem>
#define DEF_BUF_SIZE 	8192						// max size of HTTP header

typedef enum HTTPmethod_s
{
	HTTP_GET,
	HTTP_POST,
	HTTP_DELETE,
} HTTPmethod;

typedef struct HTTPurl_f
{
	std::string				scheme;
	std::string				host;
	int						port;
	std::filesystem::path	path;
	dict					query;
	std::string 			queryRaw;
	std::string 			fragment;

} HTTPurl;

class HTTPrequest : public HTTPstruct
{
	public:
		HTTPrequest( void ) : HTTPstruct() , _maxBodySize(0) {};
		virtual ~HTTPrequest( void ) override {};

		void		readHead( int socket=-1 );
		void		readPlainBody( size_t );
		void		readChunkedBody( void );
		void		parseHead( std::string const& );
		void		parseBody( void );
		bool		isCGI( void ) const noexcept;
		void		setMaxBodySize( size_t ) noexcept;

		std::string	toString( void ) const noexcept override;

		HTTPmethod		 	getMethod( void ) const noexcept;
		std::string		 	getStrMethod( void ) const noexcept;
		std::string		 	getPath( void ) const noexcept;
		std::string		 	getHost( void ) const noexcept;
		std::string	const& 	getBody( void ) const noexcept;
		std::string	const&	getQueryRaw( void ) const noexcept;

	protected:
		HTTPmethod	_method;
		HTTPurl		_url;
		std::string	_tmpBody;

		size_t		_maxBodySize;

		void	_setHead( std::string const& ) override;
		void	_setHeaders( std::string const& ) override;

		void	_setMethod( std::string const& );
		void	_setURL( std::string const& );
		void	_setVersion( std::string const& );

		void	_setScheme( std::string const& );
		void	_setHostPort( std::string const& );
		void	_setPath( std::string const& );
		void	_setQuery( std::string const& );
		void	_setFragment( std::string const& );

		void	_unchunkBody( std::string const& );
};
