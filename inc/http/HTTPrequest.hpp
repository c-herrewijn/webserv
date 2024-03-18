/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 17:05:42 by faru          #+#    #+#                 */
/*   Updated: 2024/03/18 05:10:06 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/types.h>        // send, recv
#include <sys/socket.h>       // send, recv
#include <fcntl.h>
#include <cstring>           // bzero
#include <fstream>

#include "HTTPstruct.hpp"
#include "ConfigServer.hpp"
#include "RequestValidate.hpp"

#define MAIN_PAGE_PATH	 		t_path("var/www/index.html")
#define FAVICON_PATH			t_path("var/www/favicon.ico")
#define MAX_HEADER_SIZE			8192


typedef enum HTTPreqState_f
{
	HTTP_REQ_HEAD_READING,
	HTTP_REQ_PARSING,
	HTTP_REQ_VALIDATING,
	HTTP_REQ_BODY_READING,
	HTTP_REQ_RESP_WAITING,
	HTTP_REQ_FULLFILLED,
}	HTTPreqState;

typedef struct HTTPurl_f
{
	std::string	scheme;
	std::string	host;
	int			port;
	t_path		path;
	t_dict		query;
	std::string queryRaw;
	std::string fragment;

} HTTPurl;

class HTTPrequest : public HTTPstruct
{
	public:
		HTTPrequest( int socket ) : HTTPstruct(socket, HTTP_STATIC) ,
			_state(HTTP_REQ_HEAD_READING),
			_contentLength(0) ,
			_contentLengthRead(0),
			_endConn(false) {};
		virtual ~HTTPrequest( void ) override {};

		void		parseHead( void );
		void		parseBody( void );
		void		validate( ConfigServer const& );
		std::string	toString( void ) const noexcept override;

		HTTPreqState		getState( void ) const noexcept;
		std::string		 	getMethod( void ) const noexcept;
		t_path const&		getPath( void ) const noexcept;
		std::string const&	getHost( void ) const noexcept;
		std::string		 	getPort( void ) const noexcept;
		std::string	const& 	getBody( void ) const noexcept;
		std::string	const&	getQueryRaw( void ) const noexcept;
		std::string			getContentTypeBoundary( void ) const noexcept;
		t_path				getRealPath( void ) const noexcept;
		// t_path const&	getRealPath( void ) const noexcept;
		t_path const&		getRoot( void ) const noexcept;
		t_string_map const&	getErrorPages( void ) const noexcept;
		bool				isDoneReadingHead( void ) const noexcept;
		bool				isDoneReadingBody( void ) const noexcept;
		bool				isEndConn( void ) const noexcept;

	protected:
		HTTPreqState	_state;
		HTTPmethod		_method;
		HTTPurl			_url;
		RequestValidate	_validator;

		size_t		_contentLength, _contentLengthRead;
		bool		_endConn;

		void	_readHead( void );
		void	_readPlainBody( void );
		void	_readChunkedBody( void );
		void	_setHead( std::string const& );
		void	_setHeaders(std::string const& ) override;
		void	_setType( void );

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
