/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:40:36 by fra           #+#    #+#                 */
/*   Updated: 2024/01/23 11:29:09 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include "Exception.hpp"
#include "HTTPstructs.hpp"
#include "define.hpp"
#include "WebServer.hpp"

// NB: OPEN POINTS:
//	- chunked requests
//	- relative URLs
//	- update host & port when they're found in the headers
// NB the parsing also depends on the parameters of the config file
class HTTPparser
{
	public:
		static void	parseRequest( std::string, HTTPrequest& );
		static void	printData( HTTPrequest ) noexcept;
		~HTTPparser( void ) noexcept {};

	private:
		static void	_setHead( std::string, HTTPheadReq& );
		static void	_setHeaders( std::string, dict& );
		static void	_setBody( std::string, std::string& );

		static void	_setURL( std::string, HTTPurl& );
		static void	_setVersion( std::string, HTTPversion& );

		static void	_setScheme( std::string, std::string& );
		static void	_setDomainPort( std::string, HTTPurl& );
		static void	_setPath( std::string, std::string& );
		static void	_setQuery( std::string, dict& );

		HTTPparser( void ) noexcept {};
		HTTPparser( HTTPparser const& ) noexcept;
		HTTPparser& operator=( HTTPparser const& ) noexcept;
};
