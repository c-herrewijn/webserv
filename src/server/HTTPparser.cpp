/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:47:41 by fra           #+#    #+#                 */
/*   Updated: 2024/01/31 12:48:53 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPparser.hpp"

HTTPrequest	HTTPparser::parseRequest( std::string strReq )
{
	HTTPrequest req;
	std::string head, headers, body;
	size_t		delimiter;

	delimiter = strReq.find(HTTP_TERM);
	if (delimiter == std::string::npos)
		throw(ParserException({"invalid request: no terminator"}));
	head = strReq.substr(0, delimiter + 2);
	if (delimiter + 4 != strReq.size())		// we have the body
		body = strReq.substr(delimiter + 4);
	delimiter = head.find(HTTP_NL);
	if (delimiter + 2 != head.size())		// we have the headers
	{
		headers = head.substr(delimiter + 2);
		head = head.substr(0, delimiter + 2);
	}
	_setHead(head, req);
	_setHeaders(headers, req);
	_setBody(body, req);
	return (req);
}

void	HTTPparser::_setHead(std::string header, HTTPrequest &req )
{
	std::istringstream	stream(header);
	std::string 		method, url, version;

	if (! std::getline(stream, method, ' '))
		throw(ParserException({"invalid request - invalid header:", header.c_str()}));
	req.head.method = strToMeth(method);
	if (! std::getline(stream, url, ' '))
		throw(ParserException({"invalid request - invalid header:", header.c_str()}));
	_setURL(url, req.head.url);
	if (! std::getline(stream, version, ' '))
		throw(ParserException({"invalid request - invalid header:", header.c_str()}));
	_setVersion(version, req.head.version);
	if (version.substr(version.size() - 2) != HTTP_NL)
		throw(ParserException({"invalid request - no termination header:", header.c_str()}));
}

void	HTTPparser::_setHeaders( std::string headers, HTTPrequest &req )
{
	size_t del1, del2;
	std::string key, value;

	if (headers.empty())
		return ;
	del1 = headers.find(HTTP_NL);
	do
	{
		del2 = headers.find(": ");
		if (del2 == std::string::npos)
			throw(ParserException({"invalid request - invalid header format:", headers.c_str()}));
		key = headers.substr(0, del2);
		value = headers.substr(del2 + 2, del1 - del2 - 2);
		req.headers.insert({key, value});
		headers = headers.substr(del1 + 2);
		del1 = headers.find(HTTP_NL);
	} while (del1 != std::string::npos);
	if (req.head.url.host == "")
	{
		try {
			_setHostPort(req.headers.at("Host"), req.head.url);
		}
		catch(const std::out_of_range& e) {
			throw(ParserException({"invalid request: no Host header"}));
		}
	}
}

void	HTTPparser::_setBody( std::string startBody, HTTPrequest &req )
{
	bool	isChunked = false;

	if (startBody.empty())
		return ;
	try {
		req.headers.at("Content-Type");
		try {
			req.headers.at("Content-Length");
		}
		catch(const std::out_of_range& e) {
			if (req.headers["Transfer-Encoding"] == "chunked")
				isChunked = true;
			else
				throw(ParserException({"invalid request: no Content-Length header"}));
		}
	}
	catch(const std::out_of_range& e) {
		throw(ParserException({"invalid request: no Content-Type header"}));
	}
	if (isChunked)
		_setChunked(startBody, req.body);
	else
		_setPlainBody(startBody, req);
}

void	HTTPparser::_setURL( std::string strURL, HTTPurl& url )
{
	size_t	delimiter;

	delimiter = strURL.find("://");
	if (delimiter != std::string::npos)	// scheme is not implied
	{
		_setScheme(strURL.substr(0, delimiter), url.scheme);
		strURL = strURL.substr(delimiter + 3);
	}
	delimiter = strURL.find('/');
	if (delimiter == std::string::npos)
		throw(ParserException({"invalid request - invalid url:", strURL.c_str()}));
	else if (delimiter != 0)		// there's the domain (and port)
	{
		_setHostPort(strURL.substr(0, delimiter), url);
		strURL = strURL.substr(delimiter);
	}
	_setPath(strURL, url.path);
	delimiter = strURL.find('?');
	if (delimiter != std::string::npos)
		_setQuery(strURL.substr(delimiter), url.query);
}

void	HTTPparser::_setScheme( std::string strScheme, std::string& scheme)
{
	std::transform(strScheme.begin(), strScheme.end(), strScheme.begin(), ::toupper);
	if ((strScheme != HTTP_SCHEME) and (strScheme != HTTPS_SCHEME))
		throw(ParserException({"invalid request - unsupported scheme:", strScheme.c_str()}));
	std::transform(strScheme.begin(), strScheme.end(), strScheme.begin(), ::tolower);
	scheme = strScheme;
}

void	HTTPparser::_setHostPort( std::string strURL, HTTPurl& url)
{
	size_t delimiter = strURL.find(':');
	// std::cout << url.host << '\n';
	if (delimiter != std::string::npos)	// there's the port
	{
		url.host = strURL.substr(0, delimiter);
		try {
			url.port = std::stoi(strURL.substr(delimiter + 1));
		}
		catch(const std::exception& e ) {
			throw(ParserException({"invalid request - invalid port format:", strURL.substr(delimiter + 1).c_str()}));
		}
	}
	else
	{
		url.host = strURL;
		url.port = HTTP_DEF_PORT;
	}
}

void	HTTPparser::_setPath( std::string strPath, std::string& path)
{
	path = strPath.substr(0, strPath.find('?'));
}
	
void	HTTPparser::_setQuery( std::string queries, dict& queryDict)
{
	std::string			key, value, keyValue=queries;
	size_t 				del1, del2;

	if (queries == "?")
		throw(ParserException({"invalid request - empty query"}));
	else
	while (true) 
	{
		keyValue = keyValue.substr(1);	// remove leading '?' or '&'
		del1 = keyValue.find('=');
		if (del1 == std::string::npos)
			throw(ParserException({"invalid request - invalid query:", keyValue.c_str()}));
		del2 = keyValue.find('&');
		key = keyValue.substr(0, del1);
		value = keyValue.substr(del1 + 1, del2 - del1 - 1);
		if (key.empty() or value.empty())
			throw(ParserException({"invalid request - invalid query:", keyValue.c_str()}));
		queryDict.insert({key, value});
		if (del2 == std::string::npos)
			break;
		keyValue = keyValue.substr(del2);
    }
}

void	HTTPparser::_setVersion(std::string strVersion, HTTPversion& version) 
{
	size_t	del1, del2;

	del1 = strVersion.find('/');
	if (del1 == std::string::npos)
		throw(ParserException({"invalid request - invalid version:", strVersion.c_str()}));
	version.scheme = strVersion.substr(0, del1);
	if (version.scheme != HTTP_SCHEME)
		throw(ParserException({"invalid request - invalid scheme:", strVersion.c_str()}));
	del2 = strVersion.find('.');
	if (del2 == std::string::npos)
		throw(ParserException({"invalid request - invalid version:", strVersion.c_str()}));
	try {
		version.major = std::stoi(strVersion.substr(del1 + 1, del2 - del1 - 1));
		version.minor = std::stoi(strVersion.substr(del2 + 1));
	}
	catch (std::exception const& e) {
		throw(ParserException({"invalid request - invalid version numbers:", strVersion.c_str()}));
	}
	if (version.major + version.minor != 2)
		throw(ParserException({"invalid request - unsupported HTTP version:", strVersion.c_str()}));
}

void	HTTPparser::_setChunked( std::string chunkedBody, std::string& body)
{
	size_t	sizeChunk=0, delimiter=0;

	do
	{
		delimiter = chunkedBody.find(HTTP_NL, delimiter);
		if (delimiter == std::string::npos)
			throw(ParserException({"invalid request: bad chunking"}));
		try {
			sizeChunk = std::stoul(chunkedBody.substr(0, delimiter), nullptr, 16);
		}
		catch(const std::exception& e){
			throw(ParserException({"invalid request: bad chunking"}));
		}
		body += chunkedBody.substr(delimiter + 2, sizeChunk);
		delimiter = chunkedBody.find(HTTP_NL, delimiter);
		if (delimiter == std::string::npos)
			throw(ParserException({"invalid request: bad chunking"}));
	} while (sizeChunk != 0);
}

void	HTTPparser::_setPlainBody( std::string strBody, HTTPrequest& req)
{
	size_t	delimiter = 0;

	delimiter = strBody.find(HTTP_TERM);
	if (delimiter == std::string::npos)
		throw(ParserException({"invalid request: no body terminator"}));
	req.body = strBody.substr(0, delimiter);
	try {
		if (req.body.size() != std::stoul(req.headers["Content-Length"]))
			throw(ParserException({"invalid request: body lengths do not match"}));
	}
	catch(const std::invalid_argument& e ) {
		throw(ParserException({"invalid Content-Length:", req.headers["Content-Length"].c_str()}));
	}
	catch(const std::out_of_range& e ) {
		throw(ParserException({"invalid Content-Length:", req.headers["Content-Length"].c_str()}));
	}
}

HTTPparser::HTTPparser( HTTPparser const& other ) noexcept
{
	(void) other;
}

HTTPparser& HTTPparser::operator=( HTTPparser const& other ) noexcept
{
	(void) other;
	return (*this);
}
