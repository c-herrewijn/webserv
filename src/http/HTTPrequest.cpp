/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:40:04 by fra           #+#    #+#                 */
/*   Updated: 2024/03/06 17:26:43 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPrequest.hpp"
#include "CGI.hpp"

void	HTTPrequest::parseHead( void )
{
	char		buffer[DEF_BUF_SIZE];
	std::string content, head, headers;
	size_t		delimiter = std::string::npos;
	ssize_t		charsRead = -1;

	if (this->_socket == -1)
		throw(RequestException({"invalid socket"}, 500));
	bzero(buffer, DEF_BUF_SIZE);
	charsRead = recv(this->_socket, buffer, DEF_BUF_SIZE, 0);
	if (charsRead < 0 )
		throw(RequestException({"unavailable socket"}, 500));
	else if (charsRead == 0)
		throw(ServerException({"connection closed"}));
	content = std::string(buffer, buffer + charsRead);
	delimiter = content.find(HTTP_TERM);
	// std::cout << "|" << content << "|\n";
	if (delimiter == std::string::npos)
		throw(RequestException({"no header terminator in request"}, 400));
	head = content.substr(0, delimiter);
	delimiter += HTTP_TERM.size();
	if ((delimiter + 1) < content.size())
		this->_tmpBody = content.substr(delimiter);
	delimiter = head.find(HTTP_NL);
	if (delimiter != std::string::npos)
	{
		headers = head.substr(delimiter + HTTP_NL.size()) + HTTP_NL;
		head = head.substr(0, delimiter);
		HTTPstruct::_setHeaders(headers);
	}
	_setHead(head);
}

// NB: needs to change
void	HTTPrequest::parseBody( void )
{
	if (this->_hasBody == false)
		return ;
	else if (this->_isChunked == true)
		_readChunkedBody();
	else
		_readPlainBody();
}

// NB: needs to be refined
bool	HTTPrequest::isCGI( void ) const noexcept
{
	return (this->_realPath.extension().generic_string() == ".cgi");
}

bool	HTTPrequest::isChunked( void ) const noexcept
{
	return (this->_isChunked);
}

bool	HTTPrequest::isFileUpload( void ) const noexcept
{
	return (this->_isFileUpload);
}

bool	HTTPrequest::isEndConn( void ) const noexcept
{
	return (this->_endConn);
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
	strReq += getPath().generic_string();
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

path		HTTPrequest::getPath( void ) const noexcept
{
	return (this->_url.path);
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

ConfigServer const&		HTTPrequest::getConfigServer( void ) const noexcept
{
    return (*(this->_configServer));
}

void 	HTTPrequest::setConfigServer(ConfigServer const* config) noexcept
{
	this->_configServer = config;
	this->_servName = config->getPrimaryName();
}

path	HTTPrequest::getRealPath( void ) const noexcept
{
	return (this->_realPath);
}

void 	HTTPrequest::setRealPath( path realPath ) noexcept
{
	this->_realPath = realPath;
}

void	HTTPrequest::_setMaxBodySize( size_t maxSize) noexcept
{
	this->_maxBodySize = maxSize;
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

void    HTTPrequest::_setMethod( std::string const& strMethod )
{
	if (strMethod == "GET")
		this->_method = HTTP_GET;
	else if (strMethod == "POST")
		this->_method = HTTP_POST;
	else if (strMethod == "DELETE")
		this->_method = HTTP_DELETE;
	else if ((strMethod == "HEAD") or
			(strMethod == "PUT") or
			(strMethod == "PATCH") or
			(strMethod == "OPTIONS") or
			(strMethod == "CONNECT"))
		throw(RequestException({"unsupported HTTP method:", strMethod}, 501));
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

void	HTTPrequest::_setVersion( std::string const& strVersion )
{
	size_t	del1, del2;

	del1 = strVersion.find('/');
	if (del1 == std::string::npos)
		throw(RequestException({"invalid version:", strVersion}, 400));
	this->_version.scheme = strVersion.substr(0, del1);
	std::transform(this->_version.scheme.begin(), this->_version.scheme.end(), this->_version.scheme.begin(), ::toupper);
	if (this->_version.scheme != HTTP_SCHEME)
		throw(RequestException({"invalid scheme:", strVersion}, 400));
	del2 = strVersion.find('.');
	if (del2 == std::string::npos)
		throw(RequestException({"invalid version:", strVersion}, 400));
	try {
		this->_version.major = std::stoi(strVersion.substr(del1 + 1, del2 - del1 - 1));
		this->_version.minor = std::stoi(strVersion.substr(del2 + 1));
	}
	catch (std::exception const& e) {
		throw(RequestException({"invalid version numbers:", strVersion}, 400));
	}
	if (this->_version.major + this->_version.minor != 2)
		throw(RequestException({"unsupported HTTP version:", strVersion}, 505));
}

void	HTTPrequest::checkHeaders( size_t maxBodyLength )
{
	size_t	contentLength = 0;

	if (this->_headers.count("Host") == 0)
		throw(RequestException({"no Host header"}, 412));
	if (this->_url.host == "")
		_setHostPort(this->_headers["Host"]);
	else if (this->_headers["Host"].find(this->_url.host) == std::string::npos)
		throw(RequestException({"hosts do not match"}, 412));
	if (this->_headers.count("Content-Length") == 0)
	{
		if (this->_headers.count("Transfer-Encoding") == 0)		// no body
			return ;
		else if (this->_headers.at("Transfer-Encoding") == "chunked")
		{
			if ((maxBodyLength > 0) and (this->_tmpBody.size() > maxBodyLength))
				throw(RequestException({"Content-Length is longer than maximum allowed"}, 413));
			this->_isChunked = true;
			contentLength = maxBodyLength;
		}
	}
	else
	{
		try {
			contentLength = std::stoull(this->_headers["Content-Length"]);
		}
		catch (const std::exception& e) {
			throw(RequestException({"invalid Content-Length"}, 400));
		}
		if ((maxBodyLength > 0) and (contentLength > maxBodyLength))
			throw(RequestException({"Content-Length is longer than maximum allowed"}, 413));
		else if (this->_tmpBody.size() > contentLength)
			throw(RequestException({"body is longer than expected"}, 413));
		if (this->_headers.count("Content-Type") != 0)
			this->_isFileUpload = this->_headers["Content-Type"].find("multipart/form-data; boundary=-") == 0;
	}
	this->_contentLength = contentLength;
	this->_maxBodySize = maxBodyLength;
	this->_hasBody = true;
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

//NB: add timeout error: 408
void	HTTPrequest::_readPlainBody( void )
{
    ssize_t 	readChar = -1;
    char        buffer[DEF_BUF_SIZE + 1];
	std::string	body = this->_tmpBody;
	size_t		countChars = this->_tmpBody.length();

	if (this->_socket == -1)
		throw(RequestException({"invalid socket"}, 500));
	while (countChars < this->_contentLength)
	{
		bzero(buffer, DEF_BUF_SIZE + 1);
		readChar = recv(this->_socket, buffer, DEF_BUF_SIZE, 0);
		if (readChar <= 0)
			continue;
		countChars += readChar;
		if (countChars > this->_contentLength)
			throw(RequestException({"content body is longer than expected"}, 413));
		body += buffer;
	}
	HTTPstruct::_setBody(body);
}

//NB: add timeout error: 408
void	HTTPrequest::_readChunkedBody( void )
{
    ssize_t 	readChar = -1;
    char    	buffer[DEF_BUF_SIZE + 1];
	std::string body = this->_tmpBody;
	size_t		delimiter=0, countChars=this->_tmpBody.length();

	if (this->_socket == -1)
		throw(RequestException({"invalid socket"}, 500));
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
	body = _unchunkBody(body.substr(0, delimiter + HTTP_TERM.size()));
	HTTPstruct::_setBody(body);
}
