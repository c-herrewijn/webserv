/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:40:04 by fra           #+#    #+#                 */
/*   Updated: 2024/02/19 22:30:28 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPrequest.hpp"

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

void	HTTPrequest::parseBody( size_t maxBodyLength )
{

	_checkBodyInfo(maxBodyLength);
	if (this->_hasBody == false)
		return ;
	else if (this->_isChunked == true)
		readChunkedBody();
	else
		readPlainBody();
	// std::cout << toString();
}

//NB: add timeout
void	HTTPrequest::readHead( int socket )
{
	char		buffer[DEF_BUF_SIZE + 1];
	std::string content;
	size_t		httpTerm = std::string::npos;

	_setSocket(socket);
	while (true)
	{
		bzero(buffer, DEF_BUF_SIZE + 1);
		recv(this->_socket, buffer, DEF_BUF_SIZE, 0);
		content += std::string(buffer);
		httpTerm = content.find(HTTP_TERM);
		if ( httpTerm != std::string::npos)
			break;
	}
	httpTerm += HTTP_TERM.size();
	parseHead(content.substr(0, httpTerm));
	if ((httpTerm - 1) < content.size())
		this->_tmpBody = content.substr(httpTerm);
}

//NB: add timeout error: 408
void	HTTPrequest::readPlainBody( void )
{
    ssize_t 	readChar = -1;
    char        buffer[DEF_BUF_SIZE + 1];
	std::string	body = this->_tmpBody;

	if (this->_socket == -1)
		throw(RequestException({"invalid socket"}, 500));
	while (body.size() < this->_contentLength)
	{
		bzero(buffer, DEF_BUF_SIZE + 1);
		readChar = recv(this->_socket, buffer, DEF_BUF_SIZE, 0);
		if (readChar <= 0)
			continue;
		body += buffer;
	}
	if (body.size() > this->_contentLength)
		throw(RequestException({"content body is longer than expected"}, 400));
	HTTPstruct::_setBody(body);
}

//NB: add timeout error: 408
void	HTTPrequest::readChunkedBody( void )
{
    ssize_t 	readChar = -1;
	size_t		delimiter=0, countChars=0;
    char    	buffer[DEF_BUF_SIZE + 1];
	std::string body = this->_tmpBody;

	if (this->_socket == -1)
		throw(RequestException({"invalid socket"}, 500));
	else if ((this->_maxBodySize != 0) and (this->_maxBodySize < body.size()))
		throw(RequestException({"content body is longer than the maximum allowed"}, 413));
	do
	{
		bzero(buffer, DEF_BUF_SIZE + 1);
		readChar = recv(this->_socket, buffer, DEF_BUF_SIZE, 0);
		if (readChar <= 0)
			continue;
		countChars += readChar;
		if ((this->_maxBodySize != 0) and (countChars > this->_maxBodySize))
			throw(RequestException({"content body is longer than the maximum allowed"}, 413));
		body += buffer;
		delimiter = std::string(buffer).find(HTTP_TERM);
	} while (delimiter != std::string::npos);
	_unchunkBody(body.substr(0, delimiter + HTTP_TERM.size()));
	HTTPstruct::_setBody(body);
}

// NB: needs to be refined
bool	HTTPrequest::isCGI( void ) const noexcept
{
	return (this->_url.path.extension().generic_string() == ".cgi");
}

bool	HTTPrequest::isChunked( void ) const noexcept
{
	return (this->_isChunked);
}

bool	HTTPrequest::isFileUpload( void ) const noexcept
{
	return (this->_isFileUpload);
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

HTTPmethod		HTTPrequest::getMethod( void ) const noexcept
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

std::string		HTTPrequest::getContentTypeBoundary( void ) const noexcept
{
	std::string boundary = "";
	if (this->_headers.count("Content-Type") > 0)
	{
		std::string contentType = this->_headers.at("Content-Type");
		size_t delim = contentType.find('=');
		boundary = contentType.substr(delim + 1, contentType.length() - delim);
	}
	return (boundary);
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

void	HTTPrequest::_checkBodyInfo( size_t maxBodyLength )
{
	this->_maxBodySize = maxBodyLength;
	
	try {
		this->_headers.at("Content-Length");
		try {
			this->_contentLength = std::stoull(this->_headers["Content-Length"]);
		}
		catch (const std::exception& e) {
			throw(RequestException({"invalid Content-Length"}, 400));
		}
		if ((this->_maxBodySize > 0) and (this->_contentLength > this->_maxBodySize))
			throw(RequestException({"Content-Length is longer than the maximum allowed"}, 413));
		else if (this->_tmpBody.size() > this->_contentLength)
			throw(RequestException({"body is longer than expected"}, 400));
		try
		{
			if (this->_headers.at("Content-Type").find("multipart/form-data; boundary=-") == 0)
				this->_isFileUpload = true;
		}
		catch(const std::exception& e) {}
		
	}
	catch (const std::out_of_range& e1) {
		try {
			if (this->_headers.at("Transfer-Encoding") == "chunked")
			{
				if ((this->_maxBodySize > 0) and (this->_tmpBody.size() > this->_maxBodySize))
					throw(RequestException({"Content-Length is longer than the maximum allowed"}, 413));
				this->_isChunked = true;
				this->_contentLength = this->_maxBodySize;
			}
		}
		catch(const std::out_of_range& e2) {
			this->_hasBody = false;	// no body
		}
	}
}

std::string	HTTPrequest::_unchunkBody( std::string const& chunkedBody)
{
	size_t		sizeChunk=0, delimiter=0;
	std::string	tmpChunkedBody = chunkedBody;

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
	return (tmpChunkedBody);
}
