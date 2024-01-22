/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPbuilder.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/22 18:43:11 by fra           #+#    #+#                 */
/*   Updated: 2024/01/22 23:43:55 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include <ctime>
#include "Exception.hpp"
#include "HTTPstructs.hpp"
#define HTTP_DEF_PORT		std::string("80")			// default port	- 				NB has to be set from config file!
#define HTTP_SCHEME			std::string("HTTP")			
#define HTTPS_SCHEME		std::string("HTTPS")
#define HTTP_TERM			std::string("\r\n\r\n")		// http terminator
#define HTTP_NL				std::string("\r\n")			// http delimiter
#define HTTP_SP				std::string(" ")			// shortcut for space

class HTTPbuilder
{
	public:
		~HTTPbuilder( void ) noexcept {};
		static HTTPresponse		buildResponse( int, std::string& body );
		// static std::string	respToString( HTTPresponse& ) noexcept;

	private:
		static std::string	_mapStatus( int );
		static void			_addHeader(HTTPresponse&, std::string const&, std::string const&);
		static std::string	_getDateTime( void );
		HTTPbuilder( void ) noexcept {};
		HTTPbuilder( HTTPbuilder const& ) noexcept;
		HTTPbuilder& operator=( HTTPbuilder const& ) noexcept;	
};
