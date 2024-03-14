/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 17:05:42 by faru          #+#    #+#                 */
/*   Updated: 2024/03/14 01:30:21 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/types.h>        // send, recv
#include <sys/socket.h>       // send, recv
#include <fcntl.h>
#include <cstring>           // bzero
#include <fstream>
#include <chrono>

#include "HTTPstruct.hpp"
#include "ConfigServer.hpp"
#include "RequestValidate.hpp"

#define MAIN_PAGE_PATH	 		t_path("var/www/mainPage.html")
#define FAVICON_PATH			t_path("var/www/favicon.ico")
#define MAX_TIMEOUT				5
#define MAX_HEADER_SIZE			8192

using namespace std::chrono;

typedef struct HTTPurl_f
{
	std::string	scheme;
	std::string	host;
	int			port;
	t_path		path;
	dict		query;
	std::string queryRaw;
	std::string fragment;

} HTTPurl;

class HTTPrequest : public HTTPstruct
{
	public:
		HTTPrequest( int socket ) :
			HTTPstruct(socket) ,
			_contentLength(0) ,
			_isChunked(false),
			_endConn(false),
			_gotFullHead(false) {};
		virtual ~HTTPrequest( void ) override {};

		void		readHead( void );
		void		readPlainBody( void );
		void		readChunkedBody( void );
		// void		parseBody( void );
		void		validateRequest( ConfigServer const& );
		std::string	toString( void ) const noexcept override;

		std::string		 	getMethod( void ) const noexcept;
		t_path const&		getPath( void ) const noexcept;
		std::string const&	getHost( void ) const noexcept;
		std::string		 	getPort( void ) const noexcept;
		std::string	const& 	getBody( void ) const noexcept;
		std::string	const&	getQueryRaw( void ) const noexcept;
		std::string			getContentTypeBoundary( void ) const noexcept;
		t_path const&		getRealPath( void ) const noexcept;
		t_path const&		getRoot( void ) const noexcept;
		t_string_map const&	getErrorPages( void ) const noexcept;
		bool				gotFullHead( void ) const noexcept;
		bool				isAutoIndex( void ) const noexcept;
		bool				isChunked( void ) const noexcept;
		bool				isEndConn( void ) const noexcept;

	protected:
		HTTPmethod		_method;
		HTTPurl			_url;
		RequestValidate	_validator;

		size_t		_contentLength;
		bool		_isChunked, _endConn, _gotFullHead;

		void	_parseHeads( std::string&, std::string& );
		void	_setHead( std::string const& );
		void	_setHeaders(std::string const& ) override;

		void	_setMethod( std::string const& );
		void	_setURL( std::string const& );
		void	_setScheme( std::string const& );
		void	_setHostPort( std::string const& );
		void	_setPath( std::string const& );
		void	_setQuery( std::string const& );
		void	_setFragment( std::string const& );
		void	_setVersion( std::string const& );

		void		_checkMaxBodySize( size_t );
		std::string	_unchunkBody( std::string const& );
};
