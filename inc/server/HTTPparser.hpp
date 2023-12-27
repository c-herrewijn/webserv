/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.42.fr>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:40:36 by fra           #+#    #+#                 */
/*   Updated: 2023/12/28 00:39:48 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "global.hpp"
#define HEADER_MAX_SIZE 8192	        // max size of HTTP header
#define HTTP_DEF_VERSION	"HTTP/1.1"
#define HTTP_DEF_PORT		"80"
#define HTTP_DEF_SCHEME		"http"

typedef std::map<std::string, std::string> dict;

// typedef enum HTTPparseStatus_s
// {
//     FMT_OK,
//     FMT_BADFMT,			//"empty header"
//     FMT_BIGHEAD,		//"header exceeds 8 KB maximum"
//     FMT_BADOPT,			//"bad format option header line"
// } HTTPparseStatus_t;

typedef enum HTTPreq_s
{
    HTTP_GET,
	HTTP_POST,
	HTTP_DELETE,
} HTTPreq_t;

typedef struct HTTPurl_f
{
	std::string				host;
	std::string				port;
	std::filesystem::path 	path;
	dict					query;
} HTTPurl_t;

typedef struct HTTPversion_f
{
	std::string	scheme;
	int			major;
	int			minor;
} HTTPversion_t;

typedef struct HTTPheadReq_f
{
	HTTPreq_t		method;
	HTTPurl_t		url;		
	HTTPversion_t	version;
} HTTPheadReq_t;

typedef struct HTTPheadResp_f
{
	HTTPversion_t	version;
	std::string		status;
	int				exitCode;
} HTTPheadResp_t;

typedef struct HTTPrequest_f
{
	HTTPheadReq_t	head;
	dict 			headers;
	std::string		body;
} HTTPrequest_t;

typedef struct HTTPresponse_f
{
	HTTPheadResp_t	head;
	dict 			headers;
	std::string		body;
} HTTPresponse_t;

class ParserException : std::exception
{
	public:
		ParserException( std::initializer_list<const char*> prompts) noexcept;
		virtual const char* what() const noexcept override {return (this->_msg.c_str());}
		virtual ~ParserException( void ) noexcept {}
	
	private:
		std::string _msg;
};

class HTTPparser
{
	public:
		static void	parseRequest( std::string, HTTPrequest_t& );
		// static void	printData( HTTPrequest_t* ) noexcept;

	private:
		static void	_setHead( std::string, HTTPheadReq_t& );
		static void	_setHeaders( std::string, dict& );
		static void	_setBody( std::string, std::string& );
		static void	_setMethod( std::string, HTTPreq_t& );
		static void	_setURL( std::string, HTTPurl_t& );
		static void	_setVersion( std::string, HTTPversion_t& );
		// static void	_setHostPort( HTTPrequest_t& );
		static void	_setQuery( std::string, dict& );

		HTTPparser( void ) noexcept {};
		~HTTPparser( void ) noexcept;
		HTTPparser( HTTPparser const& ) noexcept;
		HTTPparser& operator=( HTTPparser const& ) noexcept;
};
#include "WebServer.hpp"
