/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPresponse.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:01:20 by fra           #+#    #+#                 */
/*   Updated: 2024/03/12 17:10:59 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <ctime>
#include <cstring>				// bzero
#include <sys/types.h>        	// send, recv
#include <sys/socket.h>       	// send, recv
#include <unistd.h>				// read


#include "HTTPstruct.hpp"
#define STD_CONTENT_TYPE "text/html; charset=utf-8"
#define ICO_CONTENT_TYPE "image/vnd.microsoft.icon"
#define ERROR_500_CONTENT "<!DOCTYPE html>\r\n<html>\r\n\t<head>\r\n\t\t<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\r\n\t\t<title>500 - Internal Server Error</title>\r\n\t</head>\r\n\r\n\t<body>\r\n\t\t<div id=\"app\">\r\n\t\t\t<div>500</div>\r\n\t\t\t<div class=\"txt\">\r\n\t\t\t\tInternal Server Error<span class=\"blink\"></span>\r\n\t\t\t</div>\r\n\t\t\t<a href=\"/\">go home</a>\r\n\t\t</div>\r\n\t</body>\r\n</html>"

class HTTPresponse : public HTTPstruct
{
	public:
		HTTPresponse( int socket ) :
			HTTPstruct(socket) ,
			_statusCode(200),
			_HTMLfd(-1),
			_gotFullContent(false),
			_responseDone(false),
			_contentType(STD_CONTENT_TYPE) {};

		virtual ~HTTPresponse( void ) override {};
		void		parseFromStatic( void );
		void		parseFromCGI( std::string const& );
		void		readHTML( void );
		void		readContentDirectory( t_path const&);
		void		writeContent( void ) ;
		void		updateContentType( std::string ) noexcept;
		void		updateStatic500( void ) noexcept;
		std::string	toString( void ) const noexcept override;

		void		setStatusCode( int ) noexcept;
		int			getStatusCode( void ) const noexcept;
		void		setHTMLfd( int HTMLfd ) noexcept;
		int			getHTMLfd( void ) const noexcept;
		bool		isDoneReadingHTML( void ) const noexcept;
		bool		isDoneWriting( void ) const noexcept;

	protected:
		int			_statusCode, _HTMLfd;
		bool		_gotFullContent, _responseDone;
		std::string	_contentType;

		void		_setHeaders( std::string const& ) override;
		std::string	_mapStatusCode( int ) const ;
		std::string	_getDateTime( void ) const noexcept;
};
