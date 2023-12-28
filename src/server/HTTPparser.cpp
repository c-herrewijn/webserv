/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:47:41 by fra           #+#    #+#                 */
/*   Updated: 2023/12/28 19:49:47 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPparser.hpp"

ParserException::ParserException( std::initializer_list<const char*> prompts) noexcept 
	: std::exception()
{
	this->_msg = "Parsing exception - ";
	for (const char *prompt : prompts)
		this->_msg += std::string(prompt) + " ";
}

void	HTTPparser::parseRequest( std::string strReq, HTTPrequest_t &req )
{
	std::string head, headers, body;
	size_t		delimiter;

	delimiter = strReq.find(HTTP_TERM);
	if (delimiter == std::string::npos)
		throw(ParserException({"invalid request: no header terminator \\r\\n\\r\\n"}));
	head = strReq.substr(0, delimiter + 2);
	if (delimiter + 4 < strReq.size())		// there's the body request
	{
		strReq = strReq.substr(delimiter + 4);
		delimiter = strReq.find(HTTP_TERM);
		if (strReq.find(HTTP_TERM) == std::string::npos)
			throw(ParserException({"invalid request: no body terminator \\r\\n\\r\\n"}));
		body = strReq.substr(0, delimiter);
		_setBody(body, req.body);
	}
	delimiter = head.find(HTTP_DELIM);		// there are headers
	if (delimiter + 2 != head.size())
	{
		headers = head.substr(delimiter + 2);
		head = head.substr(0, delimiter + 2);
		_setHeaders(headers, req.headers);
	}
	_setHead(head, req.head);
}

void	HTTPparser::printData( HTTPrequest_t httpReq ) noexcept
{
	std::cout << "HEAD\n";
	std::cout << "\tmethod: " << httpReq.head.method << "\n";
	std::cout << "\tURL:\n\t\thost: " << httpReq.head.url.host << "\n\t\tport: " << \
		httpReq.head.url.port << "\n\t\tpath: " << httpReq.head.url.path << '\n';
	if (httpReq.head.url.query.empty() == false)
	{
		std::cout << "\t\tqueries:\n";
		for(auto option : httpReq.head.url.query)
			std::cout << "\t\t\t" << option.first << '=' << option.second << '\n';
	}
	if (httpReq.head.url.section.empty() == false)
		std::cout << "\t\tsection: " << httpReq.head.url.section << '\n';
	std::cout << "\tversion: " << httpReq.head.version.scheme << "/" << \
		httpReq.head.version.major << '.' << httpReq.head.version.minor << '\n';
	std::cout << "HEADERS\n";
	for(auto option : httpReq.headers)
		std::cout << "\t" << option.first << ": " << option.second << "\n";
	std::cout << "BODY\n\t" << httpReq.body << "\n";
}

void	HTTPparser::_setHead(std::string header, HTTPheadReq_t& head )
{
	std::istringstream	stream(header);
	std::string 		method, url, version;

	if (! std::getline(stream, method, ' '))
		throw(ParserException({"invalid header:", header.c_str()}));
	_setMethod(method, head.method);
	if (! std::getline(stream, url, ' '))
		throw(ParserException({"invalid header:", header.c_str()}));
	_setURL(url, head.url);
	if (! std::getline(stream, version, ' '))
		throw(ParserException({"invalid header:", header.c_str()}));
	_setVersion(version, head.version);
	if (version.substr(version.size() - 2) != HTTP_DELIM)
		throw(ParserException({"no termination header:", header.c_str()}));
}

void	HTTPparser::_setHeaders( std::string headers, dict& options )
{
	size_t del1, del2;
	std::string key, value;

	del1 = headers.find(HTTP_DELIM);
	do
	{
		del2 = headers.find(": ");
		if (del2 == std::string::npos)
			throw(ParserException({"invalid option format:", headers.c_str()}));
		key = headers.substr(0, del2);
		value = headers.substr(del2 + 2, del1 - del2 - 2);
		options.insert({key, value});
		headers = headers.substr(del1 + 2);
		del1 = headers.find(HTTP_DELIM);
	} while (del1 != std::string::npos);
}

void	HTTPparser::_setBody( std::string startBody, std::string& body )
{
	if (startBody.empty() == false)
		body = startBody;
}

void	HTTPparser::_setMethod(std::string strMethod, HTTPmethod_t& method) 
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
	size_t	del1, del2;

	del1 = strURL.find("http://");
	if (del1 == 0)
		strURL = strURL.substr(del1 + 7);
	del1 = strURL.find('/');
	if (del1 == std::string::npos)
		throw(ParserException({"invalid url:", strURL.c_str()}));
	else if (del1 != 0)						// there's the host
	{
		url.host = strURL.substr(0, del1);
		del2 = url.host.find(':');
		if (del2 != std::string::npos)	// there's the port
		{
			url.port = url.host.substr(del2 + 1);
			url.host = url.host.substr(0, del2);
		}
		else
			url.port = HTTP_DEF_PORT;
	}
	url.path = strURL.substr(del1);
	del1 = strURL.find('?');
	del2 = strURL.find('#');
	if (del1 != std::string::npos)
	{
		url.path = url.path.substr(0, del1);
		if (del2 != std::string::npos)
		{
			_setSection(strURL.substr(del2), url.section);
			_setQuery(strURL.substr(del1, del2 - del1), url.query);
		}
		else	
			_setQuery(strURL.substr(del1), url.query);
	}
	else if (del2 != std::string::npos)
	{
		url.path = url.path.substr(0, del2);
		_setSection(strURL.substr(del2), url.section);
	}
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

void	HTTPparser::_setSection( std::string strSection, std::string& section)
{
	section = strSection.substr(1);
}

void	HTTPparser::_setVersion(std::string strVersion, HTTPversion_t& version) 
{
	size_t	del1, del2;

	del1 = strVersion.find('/');
	if (del1 == std::string::npos)
		throw(ParserException({"invalid version:", strVersion.c_str()}));
	version.scheme = strVersion.substr(0, del1);
	if (version.scheme != HTTP_DEF_SCHEME)
		throw(ParserException({"invalid scheme:", strVersion.c_str()}));
	del2 = strVersion.find('.');
	if (del2 == std::string::npos)
		throw(ParserException({"invalid version:", strVersion.c_str()}));
	try {
		version.major = std::stoi(strVersion.substr(del1 + 1, del2 - del1 - 1));
		version.minor = std::stoi(strVersion.substr(del2 + 1));
	}
	catch (std::invalid_argument const& e) {
		throw(ParserException({"invalid version numbers:", strVersion.c_str()}));
	}
	catch (std::out_of_range const& e) {
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
