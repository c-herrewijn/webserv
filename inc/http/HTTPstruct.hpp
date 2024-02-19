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
#define HTTP_SCHEME			std::string("HTTP")			// http scheme	
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
} HTTPversion;

class HTTPstruct
{
	public:
		HTTPstruct( void ) : _socket(-1) , _hasBody(true) {}
		virtual	~HTTPstruct( void ) {};

		virtual std::string	toString( void ) const noexcept =0;

		int		getSocket( void ) const noexcept;
		bool	hasBody( void) const noexcept;

	protected:
		dict 		_headers;
		std::string	_body;
    	HTTPversion	_version;
		int			_socket;
		bool		_hasBody;

		void			_setSocket( int );
		virtual void	_setHead( std::string const& )=0;
		virtual void	_setHeaders( std::string const& );
		virtual void	_setBody( std::string const& );
		void			_setVersion( std::string const& );

		void			_addHeader(std::string const&, std::string const& ) noexcept;
	};
