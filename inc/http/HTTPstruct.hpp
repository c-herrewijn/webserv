	/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPstruct.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/22 23:36:29 by fra           #+#    #+#                 */
/*   Updated: 2024/02/09 00:22:40 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include <vector>

#include "Exceptions.hpp"

#define HTTP_DEF_PORT		80							// default port
#define HTTP_SCHEME			std::string("http")			// http scheme	
#define HTTPS_SCHEME		std::string("https")		// https scheme (maybe useless)
#define HTTP_TERM			std::string("\r\n\r\n")		// http terminator
#define HTTP_NL				std::string("\r\n")			// http delimiter
#define HTTP_SP				std::string(" ")			// shortcut for space

// void	testReqs( void );

typedef std::map<std::string, std::string> dict;

typedef struct HTTPversion_f
{
	std::string	scheme;
	int			major;
	int			minor;

	std::string	toString( void ) const;
} HTTPversion;

class HTTPstruct
{
	public:
		HTTPstruct( void );
		virtual	~HTTPstruct( void ) {};

		void				parseHead( std::string const& );
		virtual void		parseBody( std::string const& )=0;
		virtual std::string	toString( void ) const =0;
		bool				isReady( void ) const ;

	protected:
		dict 		_headers;
		std::string	_body;
    	HTTPversion	_version;
		bool		_ready;

		virtual void	_setHead( std::string const& )=0;
		virtual void	_setHeaders( std::string const& );
		virtual void	_setBody( std::string const& );
	};
