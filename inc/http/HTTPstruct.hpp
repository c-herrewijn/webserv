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

#define HTTP_DEF_PORT	80							// default port
#define HTTP_SCHEME		std::string("HTTP")			// http scheme
#define HTTP_TERM		std::string("\r\n\r\n")		// http terminator
#define HTTP_NL			std::string("\r\n")			// http delimiter
#define HTTP_SP			std::string(" ")			// shortcut for space
#define DEF_BUF_SIZE 	8192

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
		HTTPstruct( void ) : _hasBody(false) , _socket(-1) {}
		virtual	~HTTPstruct( void ) {};

		virtual std::string	toString( void ) const noexcept =0;

		bool	hasBody( void) const noexcept;
		int		getSocket( void ) const noexcept;
		void	setSocket( int ) ;

	protected:
		dict 		_headers;
		std::string	_servName, _body, _tmpBody;
    	HTTPversion	_version;
		bool		_hasBody;
		int			_socket;

		virtual void	_setHead( std::string const& )=0;
		virtual void	_setHeaders( std::string const& );
		virtual void	_setBody( std::string const& );

		void			_addHeader(std::string const&, std::string const& ) noexcept;
	};
