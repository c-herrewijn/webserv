/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:40:36 by fra           #+#    #+#                 */
/*   Updated: 2024/01/31 17:25:11 by faru          ########   odam.nl         */
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

#include <vector>
void	testReqs( void );

class HTTPparser
{
	public:
		static HTTPrequest	parseRequest( std::string );
		~HTTPparser( void ) noexcept {};

	private:
		static void	_setHead( std::string, HTTPrequest& );
		static void	_setHeaders( std::string, HTTPrequest& );
		static void	_setBody( std::string, HTTPrequest& );

		static void	_setURL( std::string, HTTPurl& );
		static void	_setVersion( std::string, HTTPversion& );

		static void	_setScheme( std::string, std::string& );
		static void	_setHostPort( std::string, HTTPurl& );
		static void	_setPath( std::string, std::string& );
		static void	_setQuery( std::string, dict&, std::string& );

		static void	_setChunkedBody( std::string, std::string& );
		static void	_setPlainBody( std::string, HTTPrequest& );
	
		HTTPparser( void ) noexcept {};
		HTTPparser( HTTPparser const& ) noexcept;
		HTTPparser& operator=( HTTPparser const& ) noexcept;
};

#include "WebServer.hpp"
