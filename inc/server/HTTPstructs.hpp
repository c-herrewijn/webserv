/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPstructs.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/22 23:36:29 by fra           #+#    #+#                 */
/*   Updated: 2024/01/22 23:48:25 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <map>
#include <string>

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
	std::string	path;
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