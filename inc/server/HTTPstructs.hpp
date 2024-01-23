/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPstructs.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/22 23:36:29 by fra           #+#    #+#                 */
/*   Updated: 2024/01/23 11:29:11 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
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

// NB: add toString() methods for every struct
typedef struct HTTPurl_f
{
	std::string	scheme;
	std::string	host;
	int			port;
	std::string	path;
	dict		query;

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

typedef struct HTTPrequest_f
{
	HTTPheadReq	head;
	dict 		headers;
	std::string	body;

	std::string	toString( void ) const ;
} HTTPrequest;

typedef struct HTTPheadResp_f
{
	HTTPversion	version;
	int			exitCode;
	std::string	status;

	std::string	toString( void ) const ;
} HTTPheadResp;

typedef struct HTTPresponse_f
{
	HTTPheadResp	head;
	dict 			headers;
	std::string		body;

	std::string	toString( void ) const ;
} HTTPresponse;