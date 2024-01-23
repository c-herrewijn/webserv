/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPbuilder.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/22 18:43:11 by fra           #+#    #+#                 */
/*   Updated: 2024/01/23 11:29:05 by faru          ########   odam.nl         */
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
#include "define.hpp"
#include "HTTPstructs.hpp"

class HTTPbuilder
{
	public:
		~HTTPbuilder( void ) noexcept {};
		static HTTPresponse		buildResponse( int, std::string& body );

	private:
		static std::string	_mapStatus( int );
		static void			_addHeader(HTTPresponse&, std::string const&, std::string const&);
		static std::string	_getDateTime( void );
		HTTPbuilder( void ) noexcept {};
		HTTPbuilder( HTTPbuilder const& ) noexcept;
		HTTPbuilder& operator=( HTTPbuilder const& ) noexcept;	
};
