/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 17:05:42 by faru          #+#    #+#                 */
/*   Updated: 2024/03/08 15:13:14 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/types.h>        // send, recv
#include <sys/socket.h>       // send, recv
#include <fcntl.h>
#include <cstring>           // strerror
#include <limits>
#include <fstream>
#include <filesystem>

#include "HTTPstruct.hpp"
#include "HTTPresponse.hpp"
#include "ConfigServer.hpp"

typedef std::filesystem::path t_path;

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
	t_path		path;
	dict		query;
	std::string queryRaw;
	std::string fragment;

} HTTPurl;

class HTTPrequest : public HTTPstruct
{
	public:
		HTTPrequest( void ) :
			HTTPstruct() ,
			_maxBodySize(0) ,
			_contentLength(0) ,
			_isChunked(false),
			_isFileUpload(false),
			_endConn(false) {};
		virtual ~HTTPrequest( void ) override {};

		void			parseHead( void );
		void			parseBody( void );
		bool			isCGI( void ) const noexcept;
		bool			isChunked( void ) const noexcept;
		bool			isFileUpload( void ) const noexcept;
		bool			isEndConn( void ) const noexcept;
		void			checkHeaders( size_t );

		std::string	toString( void ) const noexcept override;

		HTTPmethod		 	getMethod( void ) const noexcept;
		std::string		 	getStrMethod( void ) const noexcept;
		t_path			 	getPath( void ) const noexcept;
		std::string		 	getHost( void ) const noexcept;
		std::string	const& 	getBody( void ) const noexcept;
		std::string	const&	getQueryRaw( void ) const noexcept;
		std::string			getContentTypeBoundary( void ) const noexcept;
		ConfigServer const&	getConfigServer( void ) const noexcept;
		void 				setConfigServer(ConfigServer const* config) noexcept;
		t_path				getRealPath( void ) const noexcept;
		void 				setRealPath( t_path ) noexcept;

	protected:
		HTTPmethod			_method;
		HTTPurl				_url;
		ConfigServer const*	_configServer;
		t_path				_realPath;

		size_t		_maxBodySize, _contentLength;
		bool		_isChunked, _isFileUpload, _endConn;

		void	_setMaxBodySize(size_t) noexcept;
		void	_setHead( std::string const& );

		void	_setMethod( std::string const& );
		void	_setURL( std::string const& );
		void	_setScheme( std::string const& );
		void	_setHostPort( std::string const& );
		void	_setPath( std::string const& );
		void	_setQuery( std::string const& );
		void	_setFragment( std::string const& );
		void	_setVersion( std::string const& );

		std::string	_unchunkBody( std::string const& );
		void		_readPlainBody( void );
		void		_readChunkedBody( void );
};
