/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:40:36 by fra           #+#    #+#                 */
/*   Updated: 2024/02/07 09:52:54 by faru          ########   odam.nl         */
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
		static HTTPrequest	parseRequest( std::string const& );
		~HTTPparser( void ) noexcept {};

	private:
		static void	_setHead( std::string const&, HTTPrequest& );
		static void	_setHeaders( std::string const&, HTTPrequest& );
		static void	_setBody( std::string const&, HTTPrequest& );

		static void	_setURL( std::string const&, HTTPurl& );
		static void	_setVersion( std::string const&, HTTPversion& );

		static void	_setScheme( std::string const&, std::string& );
		static void	_setHostPort( std::string const&, HTTPurl& );
		static void	_setPath( std::string const&, HTTPurl& );
		static void	_setQuery( std::string const&, HTTPurl& );
		static void	_setFragment( std::string const&, std::string& );

		static void	_setChunkedBody( std::string const&, std::string& );
		static void	_setPlainBody( std::string const&, HTTPrequest& );
};

#include "WebServer.hpp"
