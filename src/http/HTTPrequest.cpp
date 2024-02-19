/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:40:04 by fra           #+#    #+#                 */
/*   Updated: 2024/02/17 17:21:32 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPrequest.hpp"

//NB: add timeout
void	HTTPrequest::readHead( int socket )
{
	char		buffer[HEADER_BUF_SIZE + 1];
	std::string content, strHead, strBody;
	size_t		endHeadPos = std::string::npos;

	if (socket != -1)
		setSocket(socket);
	while (true)
	{
		bzero(buffer, HEADER_BUF_SIZE + 1);
		recv(this->_socket, buffer, HEADER_BUF_SIZE, 0);
		content += std::string(buffer);
		if (content.find(HTTP_TERM) != std::string::npos)
			break;
	}
	endHeadPos = content.find(HTTP_TERM) + HTTP_TERM.size();
	strHead = content.substr(0, endHeadPos);
	strBody = content.substr(endHeadPos);
	parseHead(strHead);
	this->_tmpBody = strBody;
}

//NB: add timeout ?
void	HTTPrequest::readBody( size_t maxBodylength )
{
    ssize_t     	lenToRead, readChar=-1;
    char        	*buffer = nullptr;
	std::string		body;

	if (maxBodylength == 0)
		lenToRead = std::numeric_limits<ssize_t>::max();
	else if (maxBodylength < this->_tmpBody.size())
		throw(RequestException({"body length is longer than maximum allowed"}, 413));
	else if (maxBodylength == this->_tmpBody.size())
		return ;
	else
		lenToRead = maxBodylength - this->_tmpBody.size();
	buffer = new char[lenToRead + 2];
	bzero(buffer, lenToRead + 2);
	readChar = recv(this->_socket, buffer, lenToRead + 1, 0);
	this->_tmpBody += buffer;
	delete [] buffer;
	if ((maxBodylength != 0) and (readChar > (ssize_t) lenToRead))
		throw(RequestException({"body length is longer than maximum allowed"}, 413));
	parseBody();
}

void	HTTPrequest::parseHead( std::string const& strReq )
{
	std::string head, headers;
	size_t		delimiter;

	delimiter = strReq.find(HTTP_TERM);
	if (delimiter == std::string::npos)
		throw(RequestException({"no header terminator"}, 400));
	head = strReq.substr(0, delimiter);
	if (strReq.substr(delimiter) != HTTP_TERM)
		throw(RequestException({"invalid request"}, 400));
	delimiter = head.find(HTTP_NL);
	if (delimiter != std::string::npos)
	{
		headers = head.substr(delimiter + HTTP_NL.size()) + HTTP_NL;
		head = head.substr(0, delimiter);
		_setHeaders(headers);
	}
	_setHead(head);
}

void	HTTPrequest::parseBody( std::string const& strBody)
{
	bool 		isChunked=false;
	std::string	fullBody = this->_tmpBody + strBody;

    if ((fullBody.empty() == true) or (fullBody == HTTP_TERM))
		return ;
	try {
		this->_headers.at("Content-Type");
		try {
			this->_headers.at("Content-Length");
		}
		catch (const std::out_of_range& e) {
			if (this->_headers["Transfer-Encoding"] == "chunked")
				isChunked = true;
			else
				throw(RequestException({"no Content-Length header"}, 400));
		}
	}
	catch(const std::out_of_range& e) {
		throw(RequestException({"no Content-Type header"}, 400));
	}
	if (isChunked == true)
		_setChunkedBody(fullBody);
	else
		_setBody(fullBody);
}

// NB: needs to be refined
bool	HTTPrequest::isCGI( void ) const noexcept
{
	return (this->_url.path.extension().generic_string() == ".cgi");
}

std::string	HTTPrequest::toString( void ) const noexcept
{
	std::string	strReq;

	strReq += getStrMethod();
	strReq += HTTP_SP;
	strReq += this->_url.scheme;
	strReq += "://";
	strReq += this->_url.host;
	strReq += ":";
	strReq += std::to_string(this->_url.port);
	strReq += getPath();
	if (!this->_url.queryRaw.empty())
	{
		strReq += "?";
		strReq += this->_url.queryRaw;
	}
	if (!this->_url.fragment.empty())
	{
		strReq += "#";
		strReq += this->_url.fragment;
	}
	strReq += HTTP_SP;
	strReq += this->_version.scheme;
	strReq += "/";
	strReq += std::to_string(this->_version.major);
	strReq += ".";
	strReq += std::to_string(this->_version.minor);
	strReq += HTTP_NL;
	if (!this->_headers.empty())
	{
		for (auto item : this->_headers)
		{
			strReq += item.first;
			strReq += ":";
			strReq += HTTP_SP;
			strReq += item.second;
			strReq += HTTP_NL;
		}
	}
	strReq += HTTP_NL;
	if (!this->_body.empty())
		strReq += this->_body;
	return (strReq);
}

HTTPmethod			HTTPrequest::getMethod( void ) const noexcept
{
	return (this->_method);
}

std::string 	HTTPrequest::getStrMethod( void ) const noexcept
{
	switch (this->_method)
	{
		case (HTTP_GET):
			return ("GET");
		case (HTTP_POST):
			return ("POST");
		case (HTTP_DELETE):
			return ("DELETE");
		default:
			return ("");
	}
}

std::string		HTTPrequest::getPath( void ) const noexcept
{
	return (this->_url.path.generic_string());
}

std::string		HTTPrequest::getHost( void ) const noexcept
{
	std::string hostStr;
	size_t		delim;

	hostStr = this->_headers.at("Host");
	delim = hostStr.find(':');
	return (hostStr.substr(0, delim));
}

std::string	const&	HTTPrequest::getBody( void ) const noexcept
{
	return (this->_body);
}

std::string	const&	HTTPrequest::getQueryRaw( void ) const noexcept
{
	return (this->_url.queryRaw);
}

void	HTTPrequest::_setHead( std::string const& header )
{
	std::istringstream	stream(header);
	std::string 		method, url, version;

	if (! std::getline(stream, method, ' '))
		throw(RequestException({"invalid header:", header}, 400));
	_setMethod(method);
	if (! std::getline(stream, url, ' '))
		throw(RequestException({"invalid header:", header}, 400));
	_setURL(url);
	if (! std::getline(stream, version, ' '))
		throw(RequestException({"invalid header:", header}, 400));
	_setVersion(version);
}

void	HTTPrequest::_setHeaders( std::string const& headers)
{
	HTTPstruct::_setHeaders(headers);

	try {
		std::string currentHost = this->_headers.at("Host");
		if (this->_url.host == "")
			_setHostPort(currentHost);
		else if (this->_url.host != currentHost)
			throw(RequestException({"hosts do not match"}, 400));
	}
	catch(std::out_of_range const& e) {
		throw(RequestException({"no Host header"}, 400));
	}
}

void	HTTPrequest::_setBody( std::string const& strBody )
{
    std::string body = strBody;

	try {
		if (body.size() != std::stoul(this->_headers["Content-Length"]))
			throw(RequestException({"body lengths do not match"}, 400));
	}
	catch(const std::invalid_argument& e ) {
		throw(RequestException({"invalid Content-Length:", this->_headers["Content-Length"]}, 400));
	}
	catch(const std::out_of_range& e ) {
		throw(RequestException({"missing or overflow Content-Length header"}, 400));
	}
	HTTPstruct::_setBody(body);
}

void    HTTPrequest::_setMethod( std::string const& strMethod )
{
	if (strMethod == "GET")
		this->_method = HTTP_GET;
	else if (strMethod == "POST")
		this->_method = HTTP_POST;
	else if (strMethod == "DELETE")
		this->_method = HTTP_DELETE;
	else
		throw(RequestException({"unknown HTTP method:", strMethod}, 400));
}

void	HTTPrequest::_setURL( std::string const& strURL )
{
	size_t		delimiter;
	std::string	tmpURL=strURL;

	delimiter = tmpURL.find(":");
	if (delimiter != std::string::npos)
	{
		_setScheme(tmpURL.substr(0, delimiter));
		tmpURL = tmpURL.substr(delimiter + 1);
	}
	else
		_setScheme(HTTP_SCHEME);
	delimiter = tmpURL.find("//");
	if (delimiter != std::string::npos)
	{
		if (delimiter != 0)
			throw(RequestException({"bad format URL:", tmpURL}, 400));
		tmpURL = tmpURL.substr(2);
		delimiter = tmpURL.find("/");
		if (delimiter == 0)
			_setHostPort("localhost");
		else
			_setHostPort(tmpURL.substr(0, delimiter));
		tmpURL = tmpURL.substr(delimiter);
	}
	_setPath(tmpURL);
}

void	HTTPrequest::_setScheme( std::string const& strScheme )
{
	std::string	tmpScheme = strScheme;
	std::transform(tmpScheme.begin(), tmpScheme.end(), tmpScheme.begin(), ::toupper);
	if (tmpScheme != HTTP_SCHEME)
		throw(RequestException({"unsupported scheme:", strScheme}, 400));
	std::transform(tmpScheme.begin(), tmpScheme.end(), tmpScheme.begin(), ::tolower);
	this->_url.scheme = tmpScheme;
}

void	HTTPrequest::_setHostPort( std::string const& strURL )
{
	size_t 		delimiter = strURL.find(':');
	std::string	port;

	this->_url.host = strURL.substr(0, delimiter);
	if (delimiter != std::string::npos)	// there's the port
	{
		port = strURL.substr(delimiter + 1);
		if (port.empty())		// because    http://ABC.com:/%7esmith/home.html is still valid
		{
			this->_url.port = HTTP_DEF_PORT;
			return ;
		}
		try {
			this->_url.port = std::stoi(port);
		}
		catch(const std::exception& e ) {
			throw(RequestException({"invalid port format:", strURL.substr(delimiter + 1)}, 400));
		}
	}
	else
		this->_url.port = HTTP_DEF_PORT;
}

void	HTTPrequest::_setPath( std::string const& strPath )
{
	size_t		del1, del2;
	std::string	tmpPath=strPath;

	del1 = tmpPath.find('?');
	del2 = tmpPath.find('#');
	this->_url.path = tmpPath.substr(0, std::min(del1, del2));
	if (del1 != std::string::npos)
	{
		tmpPath = tmpPath.substr(del1);
		_setQuery(tmpPath);
	}
	if (del2 != std::string::npos)
	{
		tmpPath = tmpPath.substr(del2);
		_setFragment(tmpPath);
	}
}

void	HTTPrequest::_setQuery( std::string const& queries )
{
	std::string			key, value, keyValue=queries;
	size_t 				del1, del2;

	if (queries == "?")
		throw(RequestException({"empty query"}, 400));
	this->_url.queryRaw = keyValue.substr(1);
	while (true)
	{
		keyValue = keyValue.substr(1);	// remove leading '?' or '&'
		del1 = keyValue.find('=');
		if (del1 == std::string::npos)
			throw(RequestException({"invalid query:", keyValue}, 400));
		del2 = keyValue.find('&');
		key = keyValue.substr(0, del1);
		value = keyValue.substr(del1 + 1, del2 - del1 - 1);
		if (key.empty() or value.empty())
			throw(RequestException({"invalid query:", keyValue}, 400));
		this->_url.query.insert({key, value});
		if (del2 == std::string::npos)
			break;
		keyValue = keyValue.substr(del2);
	}
}

void	HTTPrequest::_setFragment( std::string const& strFragment)
{
	this->_url.fragment = strFragment.substr(1);
}

void	HTTPrequest::_setChunkedBody( std::string const& chunkedBody)
{
	size_t		sizeChunk=0, delimiter=0;
	std::string	tmpChunkedBody = this->_tmpBody + chunkedBody;

	if (chunkedBody.find(HTTP_TERM) == std::string::npos)
		throw(RequestException({"no body terminator"}, 400));
	do
	{
		delimiter = tmpChunkedBody.find(HTTP_NL);
		if (delimiter == std::string::npos)
			throw(RequestException({"bad chunking"}, 400));
		try {
			sizeChunk = std::stoul(tmpChunkedBody.substr(0, delimiter), nullptr, 16);
			this->_body += tmpChunkedBody.substr(delimiter + HTTP_NL.size(), sizeChunk);
			tmpChunkedBody = tmpChunkedBody.substr(delimiter + sizeChunk + HTTP_NL.size() * 2);
		}
		catch(const std::exception& e){
			throw(RequestException({"bad chunking"}, 400));
		}
	} while (sizeChunk != 0);
}
