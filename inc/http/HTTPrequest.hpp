/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 17:05:42 by faru          #+#    #+#                 */
/*   Updated: 2024/03/10 23:20:11 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/types.h>        // send, recv
#include <sys/socket.h>       // send, recv
#include <fcntl.h>
#include <cstring>           // strerror
#include <limits>
#include <fstream>

#include "HTTPstruct.hpp"
#include "HTTPresponse.hpp"
#include "ConfigServer.hpp"
#include "RequestValidate.hpp"

#define MAIN_PAGE_PATH	 		std::filesystem::path("var/www/test.html")
#define FAVICON_PATH			std::filesystem::path("var/www/favicon.ico")

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
			_maxBodySize(0) ,
			_contentLength(0) ,
			_isChunked(false),
			_isFileUpload(false),
			_endConn(false) {};
		virtual ~HTTPrequest( void ) override {};

		void			parseMain( void );
		void			parseBody( void );
		void			validateRequest( ConfigServer const& );
		bool			isCGI( void ) const noexcept;
		bool			isAutoIndex( void ) const noexcept;
		bool			isChunked( void ) const noexcept;
		bool			isFileUpload( void ) const noexcept;
		bool			isEndConn( void ) const noexcept;


		std::string	toString( void ) const noexcept override;

		HTTPmethod		 	getMethod( void ) const noexcept;
		std::string		 	getStrMethod( void ) const noexcept;
		t_path			 	getPath( void ) const noexcept;
		std::string const&	getHost( void ) const noexcept;
		std::string		 	getPort( void ) const noexcept;
		std::string	const& 	getBody( void ) const noexcept;
		std::string	const&	getQueryRaw( void ) const noexcept;
		std::string			getContentTypeBoundary( void ) const noexcept;
		t_path				getRealPath( void ) const noexcept;

	protected:
		HTTPmethod			_method;
		HTTPurl				_url;
		t_path				_realPath, _root;
		std::unordered_map<size_t, std::string>	_errPages;
		RequestValidate		_validator;

		size_t		_maxBodySize, _contentLength;
		bool		_isChunked, _isFileUpload, _endConn;

		void	_parseHeads( std::string&, std::string&, std::string& );
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
		void		_readPlainBody( void );
		void		_readChunkedBody( void );
};
