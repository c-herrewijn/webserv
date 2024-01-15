/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:40:36 by fra           #+#    #+#                 */
/*   Updated: 2024/01/15 19:52:19 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <sstream>
#include <initializer_list>
#include <string>
#include <map>
#include <algorithm>
#define HEADER_MAX_SIZE 	8192	        // max size of HTTP header
#define HTTP_DEF_PORT		std::string("80")			// default port
#define HTTP_SCHEME			std::string("HTTP")
#define HTTPS_SCHEME		std::string("HTTPS")
#define HTTP_TERM			std::string("\r\n\r\n")		// http terminator
#define HTTP_DELIM			std::string("\r\n")			// http delimiter
#define HTTP_SP				std::string(" ")

typedef std::map<std::string, std::string> dict;

typedef enum HTTPmethod_s
{
    HTTP_GET,
	HTTP_POST,
	HTTP_DELETE,
} HTTPmethod;

// NB: add toString() methods for every struct
typedef struct HTTPurl_f
{
	std::string	scheme;
	std::string	domain;
	std::string	port;
	std::string	path;		// std::filesystem
	dict		query;
} HTTPurl;

typedef struct HTTPversion_f
{
	std::string	scheme;
	int			major;
	int			minor;
} HTTPversion;

typedef struct HTTPheadReq_f
{
	HTTPmethod	method;
	HTTPurl		url;		
	HTTPversion	version;
} HTTPheadReq;

typedef struct HTTPrequest_f
{
	HTTPheadReq	head;
	dict 		headers;
	std::string	body;
} HTTPrequest;

typedef struct HTTPheadResp_f
{
	HTTPversion	version;
	std::string	status;
	int			exitCode;
} HTTPheadResp;

typedef struct HTTPresponse_f
{
	HTTPheadResp	head;
	dict 			headers;
	std::string		body;
} HTTPresponse;

class HTTPexception : std::exception
{
	public:
		HTTPexception( std::initializer_list<const char*> ) noexcept;
		virtual const char* what() const noexcept override {return (this->_msg.c_str());}
		virtual ~HTTPexception( void ) noexcept {}
	
	protected:
		std::string _msg;
};

class ParserException : public HTTPexception
{
	public:
		ParserException( std::initializer_list<const char*> ) noexcept;
};

class HTTPparser
{
	public:
		static void			parseRequest( std::string, HTTPrequest& );
		static void			buildResponse( HTTPresponse&, int, std::string& body );
		// static std::string	reqToString( HTTPrequest& ) noexcept;
		// static std::string	respToString( HTTPresponse& ) noexcept;
		~HTTPparser( void ) noexcept {};

	private:
		// requests
		static void	_setHead( std::string, HTTPheadReq& );
		static void	_setHeaders( std::string, dict& );
		static void	_setBody( std::string, std::string& );

		static void	_setMethod( std::string, HTTPmethod& );
		static void	_setURL( std::string, HTTPurl& );
		static void	_setVersion( std::string, HTTPversion& );

		static void	_setScheme( std::string, std::string& );
		static void	_setDomainPort( std::string, HTTPurl& );
		static void	_setPath( std::string, std::string& );
		static void	_setQuery( std::string, dict& );

		// responses
		static std::string	_mapStatus( int );
		// static std::string const&	_mapMethod( HTTPmethod );
		HTTPparser( void ) noexcept {};
		HTTPparser( HTTPparser const& ) noexcept;
		HTTPparser& operator=( HTTPparser const& ) noexcept;
};
#include "WebServer.hpp"
