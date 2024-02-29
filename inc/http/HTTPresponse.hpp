/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPresponse.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:01:20 by fra           #+#    #+#                 */
/*   Updated: 2024/02/29 17:31:44 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <ctime>
#include <cstring>				// ft_bzero
#include <sys/types.h>        	// send, recv
#include <sys/socket.h>       	// send, recv       
#include <unistd.h>				// read


#include "HTTPstruct.hpp"
#define STD_CONTENT_TYPE "text/html; charset=utf-8"

class HTTPresponse : public HTTPstruct
{
	public:
		HTTPresponse( void ) : 
			HTTPstruct() ,
			_statusCode(200) ,
			_HTMLfd(-1),
			_gotFullHTML(false),
			_writtenResp(false) {};
		virtual ~HTTPresponse( void ) override {};

		void		parseStaticHTML( int );
		void		parseFromCGI( std::string const& );
		void		readHTML( void );
		void		writeContent( void ) ;
		std::string	toString( void ) const noexcept override;

		int			getStatusCode( void ) const noexcept;
		void		setHTMLfd( int HTMLfd ) ;
		int			getHTMLfd( void ) const noexcept;
		bool		isDoneReadingHTML( void ) const noexcept;
		bool		isDoneWriting( void ) const noexcept;

	protected:
		int		_statusCode, _HTMLfd;
		bool	_gotFullHTML, _writtenResp;	

		void		_setHead( std::string const& ) override;
		void		_setHeaders( std::string const& ) override;
		std::string	_mapStatusCode( int ) const ;
		std::string	_getDateTime( void ) const noexcept;
		std::string	_getStatusFromHeader( void ) const;
};
