/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:40:36 by fra           #+#    #+#                 */
/*   Updated: 2024/01/03 16:48:42 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <sstream>
#include <initializer_list>
#include <string>
#include <map>
#define HEADER_MAX_SIZE 	8192	        // max size of HTTP header
#define HTTP_DEF_PORT		"80"			// default port
#define HTTP_DEF_SCHEME		"HTTP"			// default scheme
#define HTTP_TERM			"\r\n\r\n"		// http terminator
#define HTTP_DELIM			"\r\n"			// http delimiter
#define HTTP_SCHEME			"http"
#define HTTPS_SCHEME		"https"

typedef std::map<std::string, std::string> dict;

typedef enum HTTPmethod_s
{
    HTTP_GET,
	HTTP_POST,
	HTTP_DELETE,
} HTTPmethod;

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
		static void	parseRequest( std::string, HTTPrequest& );
		static void	printData( HTTPrequest ) noexcept;
		~HTTPparser( void ) noexcept {};

	private:
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

		HTTPparser( void ) noexcept {};
		HTTPparser( HTTPparser const& ) noexcept;
		HTTPparser& operator=( HTTPparser const& ) noexcept;
};
#include "WebServer.hpp"
