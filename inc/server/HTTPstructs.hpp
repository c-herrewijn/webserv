/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPstructs.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/22 23:36:29 by fra           #+#    #+#                 */
/*   Updated: 2024/02/08 17:28:07 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

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

class HTTPstruct
{
	public:
		static void	parse( std::string const& );
		~HTTPstruct( void ) noexcept {};

	private:
		std::map<std::string, std::string> 		headers;
		std::string								body;

		// main part
		// static void	_setHead( std::string const&, HTTPrequest& );
		void	_setHeaders( std::string const&, HTTPrequest& )=0;
		void	_setBody( std::string const&, HTTPrequest& )=0;

		// head
		// static void _setMethod( std::string );
		// static void	_setURL( std::string const&, HTTPurl& );
		// static void	_setVersion( std::string const&, HTTPversion& );

		// URL
		// static void	_setScheme( std::string const&, std::string& );
		// static void	_setHostPort( std::string const&, HTTPurl& );
		// static void	_setPath( std::string const&, HTTPurl& );
		// static void	_setQuery( std::string const&, HTTPurl& );
		// static void	_setFragment( std::string const&, std::string& );

		// body
		// static void	_setChunkedBody( std::string const&, std::string& );
		// static void	_setPlainBody( std::string const&, HTTPrequest& );
};




#include <map>
#include <string>
#include "Exception.hpp"
#include "define.hpp"

typedef std::map<std::string, std::string> dict;

typedef enum HTTPmethod_s
{
    HTTP_GET,
	HTTP_POST,
	HTTP_DELETE,
} HTTPmethod;

std::string	methToStr(HTTPmethod const&);

HTTPmethod	strToMeth(std::string const&);

typedef struct HTTPurl_f
{
	std::string	scheme;
	std::string	host;
	int			port;
	std::string	path;
	dict		query;
	std::string queryRaw;
	std::string fragment;

	std::string	toString( void ) const ;
} HTTPurl;

typedef struct HTTPversion_f
{
	std::string	scheme;
	int			major;
	int			minor;

	std::string	toString( void ) const ;
} HTTPversion;

typedef struct HTTPheadReq_f
{
	HTTPmethod	method;
	HTTPurl		url;
	HTTPversion	version;

	std::string	toString( void ) const ;
} HTTPheadReq;

// typedef struct HTTPrequest_f
// {
// 	HTTPheadReq	head;
// 	dict 		headers;
// 	std::string	body;

// 	std::string	toString( void ) const ;
// } HTTPrequest;

typedef struct HTTPheadResp_f
{
	HTTPversion	version;
	int			exitCode;
	std::string	status;

	std::string	toString( void ) const ;
} HTTPheadResp;

// typedef struct HTTPresponse_f
// {
// 	HTTPheadResp	head;
// 	dict 			headers;
// 	std::string		body;

// 	std::string	toString( void ) const ;
// } HTTPresponse;
