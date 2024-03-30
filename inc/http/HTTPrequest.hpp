#pragma once

#include <sys/types.h>        // send, recv
#include <sys/socket.h>       // send, recv
#include <cstring>           // bzero
#include <fstream>

#include "HTTPstruct.hpp"
#include "HTTPresponse.hpp"
#include "ConfigServer.hpp"
#include "RequestValidate.hpp"

#define LOCALHOST		std::string("localhost") 
#define MAX_HEADER_SIZE	8192

#define	HEADER_CONT_LEN			"Content-Length"
#define	HEADER_CONT_TYPE		"Content-Type"
#define	HEADER_HOST				"Host"
#define	HEADER_CONN				"Connection"
#define	HEADER_TRANS_ENCODING	"Transfer-Encoding"

typedef enum HTTPreqState_f
{
	HTTP_REQ_HEAD_READING,
	HTTP_REQ_HEAD_PARSING,
	HTTP_REQ_BODY_READING,
	HTTP_REQ_DONE,
}	HTTPreqState;

typedef struct HTTPurl_f
{
	std::string	scheme;
	std::string	host;
	std::string	port;
	t_path		path;
	t_dict		query;
	std::string queryRaw;
	std::string fragment;

} HTTPurl;

typedef std::vector<ConfigServer> t_serv_list;

class HTTPrequest : public HTTPstruct
{
	public:
		HTTPrequest( int socket, t_serv_list const& );
		virtual ~HTTPrequest( void ) override {};

		void			parseHead( void );
		void			parseBody( void );
		std::string		toString( void ) const noexcept override;

		std::string		 	getMethod( void ) const noexcept;
		std::string			getHost( void ) const noexcept;
		std::string		 	getPort( void ) const noexcept;
		size_t			 	getContentLength( void ) const noexcept;
		std::string	const&	getQueryRaw( void ) const noexcept;
		std::string			getContentTypeBoundary( void ) const noexcept;
		std::string const&	getServName( void ) const noexcept;
		t_path const&		getRealPath( void ) const noexcept;
		t_path const&		getRoot( void ) const noexcept;
		t_path				getErrorPageFromCode( int, t_path const& );

		bool	isRedirection( void ) const noexcept;
		bool	isEndConn( void ) noexcept;
		bool	isDoneReadingHead( void ) const noexcept;
		bool	isDoneParsingHead( void ) const noexcept;
		bool	isDoneReadingBody( void ) const noexcept;
		bool	theresBodyToRead( void ) const noexcept;

	protected:
		HTTPreqState	_state;
		HTTPmethod		_method;
		HTTPurl			_url;

		RequestValidate	_validator;
		t_serv_list		_servers;
		ConfigServer	_defaultServer, _handlerServer;

		std::string _tmpHead;
		size_t		_contentLength, _contentLengthRead, _maxBodySize;

		void	_readHead( void );
		void	_readPlainBody( void );
		void	_readChunkedBody( void );
		void	_validate( void );
		void	_setHead( std::string const& );
		void	_setHeaders(std::string const& ) override;
		void	_setHandlerServer( std::string const& ) noexcept;
		void	_checkConfig( void );
		void	_setTypeUpdateState( void );
		void	_checkMaxBodySize( size_t );

		void	_setMethod( std::string const& );
		void	_setURL( std::string const& );
		void	_setScheme( std::string const& );
		void	_setHostPort( std::string const& );
		void	_setPath( std::string const& );
		void	_setQuery( std::string const& );
		void	_setFragment( std::string const& );
		void	_setVersion( std::string const& ) override;

};
