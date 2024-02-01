/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPstructs.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/23 10:40:31 by faru          #+#    #+#                 */
/*   Updated: 2024/02/01 13:37:47 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPstructs.hpp"

std::string	methToStr(HTTPmethod const& method)
{
	switch (method)
	{
		case HTTP_GET:
			return ("GET");
		case HTTP_POST:
			return ("POST");
		case HTTP_DELETE:
			return ("DELETE");
		default:
			throw(ServerException({"Unknown HTTP method"}));
	}
}

HTTPmethod	strToMeth(std::string const& strMethod)
{
	if (strMethod == "GET")
		return (HTTP_GET);
	else if (strMethod == "POST")
		return (HTTP_POST);
	else if (strMethod == "DELETE")
		return (HTTP_DELETE);
	else
		throw(ServerException({"unknown http method:", strMethod.c_str()}));
}

std::string	HTTPurl::toString( void ) const
{
	std::string	strURL;

	strURL += this->scheme;
	strURL += "://";
	strURL += this->host;
	strURL += ":";
	strURL += std::to_string(this->port);
	strURL += this->path;
	if (!this->queryRaw.empty())
	{
		strURL += "?";
		strURL += this->queryRaw;
	}
	if (!this->fragment.empty())
	{
		strURL += "#";
		strURL += this->fragment;
	}
	return (strURL);
}

std::string	HTTPversion::toString( void ) const
{
	std::string	strVersion;
	strVersion += this->scheme;
	strVersion += "/";
	strVersion += std::to_string(this->major);
	strVersion += ".";
	strVersion += std::to_string(this->minor);
	return (strVersion);
}

std::string	HTTPheadReq::toString( void ) const
{
	std::string	strHead;

	strHead += methToStr(this->method);
	strHead += HTTP_SP;
	strHead += this->url.toString();
	strHead += HTTP_SP;
	strHead += this->version.toString();
	return (strHead);
}

std::string	HTTPrequest::toString( void ) const
{
	std::string	strReq;

	strReq += this->head.toString();
	strReq += HTTP_NL;
	if (!this->headers.empty())
	{
		for (auto item : this->headers)
		{
			strReq += item.first;
			strReq += ":";
			strReq += HTTP_SP;
			strReq += item.second;
			strReq += HTTP_NL;
		}
	}
	strReq += HTTP_NL;
	if (!this->body.empty())
	{
		strReq += this->body;
		strReq += HTTP_TERM;
	}
	return (strReq);
}

std::string	HTTPheadResp::toString( void ) const
{
	std::string	strHeadResp;

	strHeadResp += this->version.toString();
	strHeadResp += HTTP_SP;
	strHeadResp += std::to_string(this->exitCode);
	strHeadResp += HTTP_SP;
	strHeadResp += this->status;
	return (strHeadResp);
}

std::string	HTTPresponse::toString( void ) const
{
	std::string	strResp;

	strResp += this->head.toString();
	strResp += HTTP_NL;
	if (!this->headers.empty())
	{
		for (auto item : this->headers)
		{
			strResp += item.first;
			strResp += ":";
			strResp += HTTP_SP;
			strResp += item.second;
			strResp += HTTP_NL;
		}
	}
	strResp += HTTP_NL;
	if (!this->body.empty())
	{
		strResp += this->body;
		strResp += HTTP_TERM;
	}
	return (strResp);
}
