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
#include <filesystem>
#include <chrono>

#include "Exceptions.hpp"

#define HTTP_DEF_PORT	80							// default port
#define HTTP_SCHEME		std::string("HTTP")			// http scheme
#define HTTP_TERM		std::string("\r\n\r\n")		// http terminator
#define HTTP_NL			std::string("\r\n")			// http delimiter
#define HTTP_SP			' '							// shortcut for space
#define DEF_BUF_SIZE 	1024
#define MAX_TIMEOUT				5

using namespace std::chrono;

typedef std::map<std::string, std::string> t_dict;
typedef std::filesystem::path t_path;

typedef enum HTTPmethod_s
{
	HTTP_GET,
	HTTP_POST,
	HTTP_DELETE,
}	HTTPmethod;

typedef enum HTTPtype_s
{
	HTTP_STATIC,
	HTTP_AUTOINDEX_STATIC,
	HTTP_CHUNKED,
	HTTP_FAST_CGI,
	HTTP_FILE_UPL_CGI,
}	HTTPtype;

typedef struct HTTPversion_f
{
	std::string	scheme;
	int			major;
	int			minor;
} HTTPversion;

class HTTPstruct
{
	public:
		HTTPstruct( int, HTTPtype );
		virtual	~HTTPstruct( void ) {};

		virtual std::string	toString( void ) const noexcept =0;
		bool				hasBody( void ) const noexcept;

		HTTPtype			getType( void ) const noexcept;
		int					getSocket( void ) const noexcept;
		std::string const&	getServName( void ) const noexcept;
		void				setServName(std::string) noexcept;
		std::string const&	getTmpBody( void );
		virtual void		setTmpBody( std::string const& );

		bool				isCGI( void ) const noexcept;
		bool				isFastCGI( void ) const noexcept;
		bool				isFileUpload( void ) const noexcept;
		bool				isChunked( void ) const noexcept;
		bool				isAutoIndex( void ) const noexcept;

	protected:
		int			_socket;
		HTTPtype	_type;
	
		t_dict 		_headers;
		std::string	_servName, _tmpHead, _body, _tmpBody;
    	HTTPversion	_version;

		steady_clock::time_point	_lastActivity;

		virtual void	_setHeaders( std::string const& );
		virtual void	_setBody( std::string const& );

		void			_resetTimeout( void ) noexcept;
		void			_checkTimeout( void );

		void			_addHeader(std::string const&, std::string const& ) noexcept;
	};
