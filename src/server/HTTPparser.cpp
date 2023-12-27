/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:47:41 by fra           #+#    #+#                 */
/*   Updated: 2023/12/28 00:52:00 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPparser.hpp"

ParserException::ParserException( std::initializer_list<const char*> prompts) noexcept 
	: std::exception()
{
	this->_msg = "Parsing exception: ";
	for (const char *prompt : prompts)
		this->_msg += std::string(prompt) + " ";
}

void	HTTPparser::parseRequest( std::string strReq, HTTPrequest_t &req )
{
	std::string head, headers, body;
	size_t		delimiter;

	delimiter = strReq.find("\r\n\r\n");
	if (delimiter == std::string::npos)
		throw(ParserException({"invalid request: no terminator \\r\\n\\r\\n"}));
	head = strReq.substr(delimiter);
	delimiter = head.find("\r\n");
	if (delimiter != std::string::npos)
	{
		headers = head.substr(delimiter + 2, std::string::npos);
		head = head.substr(delimiter);
	}
	_setHead(head, req.head);
	_setHeaders(headers, req.headers);
	_setBody(body, req.body);
}

// void	HTTPparser::printData( HTTPrequest_t httpReq )
// {
// 	std::cout << "request: " << httpReq.request << "\n";
// 	for(auto option : httpReq.options)
// 		std::cout << "\toption: " << option.first << ": " << option.second << "\n";
// 	std::cout << "body: " << httpReq.body << "\n";
// }

void	HTTPparser::_setHead(std::string header, HTTPheadReq_t& head )
{
	std::istringstream	stream(header);
	std::string 		method, url, version, termination;

	stream >> method;
	if (! stream.good())
		throw(ParserException({"invalid header:", header.c_str()}));
	_setMethod(method, head.method);
	stream >> url;
	if (! stream.good())
		throw(ParserException({"invalid header:", header.c_str()}));
	_setURL(url, head.url);
	stream >> version;
	if (! stream.good())
		throw(ParserException({"invalid header:", header.c_str()}));
	_setVersion(version, head.version);
	stream >> termination;
	if ((! stream.good()) or (termination != "\r\n"))
		throw(ParserException({"invalid header:", header.c_str()}));
}

void	HTTPparser::_setHeaders( std::string headers, dict& options )
{
	// char *eol, *colon;
	(void) headers;
	(void) options;
	// while (true)
	// {
	// 	eol = strstr(line, "\r\n");
	// 	if (eol == nullptr)
	// 		break;
	// 	eol[0] = '\0';
	// 	colon = strchr(line, ':');
	// 	if (colon == nullptr)
	// 		return (FMT_BADOPT);
	// 	*colon = '\0';
	// 	options.insert({line, colon + 2});
	// 	line = eol + 2;
	// }
	// return (FMT_OK);
}

void	HTTPparser::_setBody( std::string startBody, std::string& body )
{
	if (startBody.empty() == false)
		body = startBody;
}

void	HTTPparser::_setMethod(std::string strMethod, HTTPreq_t& method) 
{
	if (strMethod == "GET")
		method = HTTP_GET;
	else if (strMethod == "POST")
		method = HTTP_POST;
	else if (strMethod == "DELETE")
		method = HTTP_DELETE;
	else
		throw(ParserException({"unknown http method:", strMethod.c_str()}));
}

void	HTTPparser::_setURL( std::string strURL, HTTPurl_t& url )
{
	size_t	delimiter, end=std::string::npos;

	delimiter = strURL.find("http://");
	if (delimiter != std::string::npos)
		strURL = strURL.substr(delimiter + 7, end);
	delimiter = strURL.find('/');
	if (delimiter == std::string::npos)
		throw(ParserException({"invalid header:", strURL.c_str()}));
	else if (delimiter != 0)						// there's the host
	{
		url.host = strURL.substr(delimiter);
		delimiter = url.host.find(':');
		if (delimiter != std::string::npos)	// there's the port
		{
			url.port = url.host.substr(delimiter + 1, end);
			url.host = url.host.substr(delimiter);
		}
		else
			url.port = HTTP_DEF_PORT;
	}
	end = strURL.find('?', delimiter);
	url.path = strURL.substr(delimiter, end);
	if (end != std::string::npos)
		_setQuery(strURL.substr(end, std::string::npos), url.query);
}

void	HTTPparser::_setQuery( std::string queries, dict& queryDict)
{
	std::istringstream 	stream(queries);
	std::string			keyValue, key, value;
	size_t 				divider;

	while (std::getline(stream, keyValue, '&')) 
	{
		divider = keyValue.find('=');
		if (divider == std::string::npos)
			throw(ParserException({"invalid query:", keyValue.c_str()}));
		key = keyValue.substr(divider);
		value = keyValue.substr(divider + 1, std::string::npos);
		queryDict.insert({key, value});
    }
}

void	HTTPparser::_setVersion(std::string strVersion, HTTPversion_t& version) 
{
	size_t	del1, del2;

	del1 = strVersion.find('/');
	if (del1 == std::string::npos)
		throw(ParserException({"invalid version:", strVersion.c_str()}));
	version.scheme = strVersion.substr(del1);
	if (version.scheme != HTTP_DEF_SCHEME)
		throw(ParserException({"invalid scheme:", strVersion.c_str()}));
	del2 = strVersion.find('.');
	if (del2 == std::string::npos)
		throw(ParserException({"invalid version:", strVersion.c_str()}));
	try
	{
		version.major = std::stoi(strVersion.substr(del1, del2));
		version.minor = std::stoi(strVersion.substr(del2, std::string::npos));
	}
	catch (const std::exception& e) 
	{
		throw(ParserException({"invalid version numbers:", strVersion.c_str()}));
	}
}

HTTPparser::~HTTPparser( void ) noexcept
{
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
