#pragma once
#include <ctime>
#include <cstring>				// bzero
#include <sys/types.h>        	// send, recv
#include <sys/socket.h>       	// send, recv
#include <unistd.h>				// read
#include <fcntl.h>
#include <set>
#include <cmath>

#include "HTTPstruct.hpp"

#define HTML_CONTENT_TYPE	"text/html; charset=utf-8"
#define CSS_CONTENT_TYPE	"text/css"
#define ICO_CONTENT_TYPE	"image/vnd.microsoft.icon"
#define ERROR_500_CONTENT	"<!DOCTYPE html>\r\n<html>\r\n\t<head>\r\n\t\t<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\r\n\t\t<title>500 - Internal Server Error</title>\r\n\t</head>\r\n\r\n\t<body>\r\n\t\t<div id=\"app\">\r\n\t\t\t<div>500</div>\r\n\t\t\t<div class=\"txt\">\r\n\t\t\t\tInternal Server Error<span class=\"blink\"></span>\r\n\t\t\t</div>\r\n\t\t\t<a href=\"/\">go home</a>\r\n\t\t</div>\r\n\t</body>\r\n</html>"

typedef enum HTTPrespState_f
{
	HTTP_RESP_HTML_READING,
	HTTP_RESP_PARSING,
	HTTP_RESP_WRITING,
	HTTP_RESP_DONE,
}	HTTPrespState;

class HTTPresponse : public HTTPstruct
{
	public:
		HTTPresponse( int, int, HTTPtype type=HTTP_STATIC);
		virtual ~HTTPresponse( void ) override {};

		void		parseFromCGI( std::string const& );
		void		parseFromStatic( std::string const& );
		void		readHTML( void );
		void		listContentDirectory( t_path const&);
		void		writeContent( void ) ;
		void		errorReset( int, bool hardCode ) noexcept;
		std::string	toString( void ) const noexcept override;

		int			getHTMLfd( void ) const noexcept;
		void		setTargetFile( t_path const& );
		bool		isDoneReadingHTML( void ) const noexcept;
		bool		isParsingNeeded( void ) const noexcept;
		bool		isDoneWriting( void ) const noexcept;

	protected:
		HTTPrespState	_state;
		t_path			_targetFile;
		int				_HTMLfd;
		size_t			_contentLengthWrite;
		std::string		_contentType, _strSelf;

		void		_setHeaders( std::string const& ) override;
		std::string	_mapStatusCode( int ) const ;
		std::string	_getDateTime( void ) const noexcept;
};
