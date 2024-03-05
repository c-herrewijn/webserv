/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 17:05:42 by faru          #+#    #+#                 */
/*   Updated: 2024/03/05 15:45:59 by faru          ########   odam.nl         */
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
#include <chrono>

#include "HTTPstruct.hpp"
// #include "HTTPresponse.hpp"
#include "ConfigServer.hpp"

#define MAX_TIMEOUT	5		// seconds

using namespace std::chrono;

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

class CGI;

class HTTPrequest : public HTTPstruct
{
	public:
		HTTPrequest( void ) :
			HTTPstruct() ,
			_contentLength(std::numeric_limits<size_t>::max()) ,
			_isChunked(false),
			_isFileUpload(false),
			_endConn(false), 
			_gotFullBody(false) {};
		virtual ~HTTPrequest( void ) override {};

		void			parseHead( void );
		void			parseBody( void );
		bool			isCGI( void ) const noexcept;
		bool			isChunked( void ) const noexcept;
		bool			isFileUpload( void ) const noexcept;
		bool			isEndConn( void ) const noexcept;
		void			checkHeaders( size_t );
		CGI *			cgi;

		std::string	toString( void ) const noexcept override;

		HTTPmethod		 	getMethod( void ) const noexcept;
		std::string		 	getStrMethod( void ) const noexcept;
		std::string		 	getPath( void ) const noexcept;
		std::string		 	getHost( void ) const noexcept;
		std::string	const& 	getBody( void ) const noexcept;
		std::string	const&	getQueryRaw( void ) const noexcept;
		std::string			getContentTypeBoundary( void ) const noexcept;
		std::string const&	getRoot( void ) const noexcept;
		void 				setRoot(std::string const&) noexcept;

	protected:
		HTTPmethod			_method;
		HTTPurl				_url;

		size_t		_contentLength;
		bool		_isChunked, _isFileUpload, _endConn, _gotFullBody;
		std::string	_root;

		void	_setHead( std::string const& ) override;
		void	_setMethod( std::string const& );
		void	_setURL( std::string const& );
		void	_setScheme( std::string const& );
		void	_setHostPort( std::string const& );
		void	_setPath( std::string const& );
		void	_setQuery( std::string const& );
		void	_setFragment( std::string const& );
		void	_setVersion( std::string const& );

		void	_readPlainBody( void );
		void	_readChunkedBody( void );

		std::string	_unchunkChunk( std::string const&, std::string& );
		std::string	_unchunkBody( std::string const& );
};
