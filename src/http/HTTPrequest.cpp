/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:40:04 by fra           #+#    #+#                 */
/*   Updated: 2024/02/18 03:24:14 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPrequest.hpp"

//NB: add timeout error: 408
void	HTTPrequest::readHead( int socket )
{
	char		buffer[DEF_BUF_SIZE + 1];
	std::string content, strHead, strBody;
	size_t		endHeadPos = std::string::npos;

	if (socket != -1)
		setSocket(socket);
	while (true)
	{
		bzero(buffer, DEF_BUF_SIZE + 1);
		recv(this->_socket, buffer, DEF_BUF_SIZE, 0);
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

//NB: add timeout error: 408
void	HTTPrequest::readPlainBody( size_t limitSize )
{
	size_t     	lenToRead=DEF_BUF_SIZE, count=0;
    ssize_t 	readChar = -1;
    char        *buffer = nullptr;
	std::string	body = this->_tmpBody;

	if (limitSize < this->_tmpBody.size())
		throw(RequestException({"content body is longer than the maximum allowed"}, 413));
	else if (limitSize < lenToRead)
		lenToRead = limitSize;
	buffer = new char[lenToRead + 1];
	bzero(buffer, lenToRead + 1);
	while (lenToRead)
	{
		readChar = recv(this->_socket, buffer, lenToRead, 0);
		if (readChar < 0)
			break;
		lenToRead -= readChar;
		count += readChar;
		buffer += readChar;
	}
	delete [] buffer;
	if (readChar < 0)
		throw(RequestException({"unavailable socket"}, 500));
	body = std::string (buffer - count);
	HTTPstruct::_setBody(body);
}

//NB: add timeout error: 408
void	HTTPrequest::readChunkedBody( size_t limitSize )
{
    ssize_t 	readChar=-1;
    char    	buffer[DEF_BUF_SIZE];
	size_t		delimiter=0, countChars=0;
	std::string	data, body=this->_tmpBody;

	if (limitSize == 0)
		limitSize = std::numeric_limits<std::size_t>::max();
	else if (limitSize < body.size())
		throw(RequestException({"content body is longer than the maximum allowed"}, 413));
	do
	{
		bzero(buffer, DEF_BUF_SIZE + 1);
		readChar = recv(this->_socket, buffer, DEF_BUF_SIZE, 0);
		if (readChar < 0)
			throw(RequestException({"unavailable socket"}, 500));
		countChars += readChar;
		if (countChars > limitSize)
			throw(RequestException({"content body is longer than the maximum allowed"}, 413));
		data = buffer;
		body += data;
		delimiter = data.find(HTTP_TERM);
	} while (delimiter != std::string::npos);
	_unchunkBody(body.substr(0, delimiter + HTTP_TERM.size()));
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

void	HTTPrequest::parseBody( size_t limitBodySize )
{
	size_t	bodySize = 0;
	bool 	isChunked = false;

	try {
		this->_headers.at("Content-Type");
		try {
			bodySize = std::stoull(this->_headers.at("Content-Length"));
		}
		catch (const std::out_of_range& e1) {
			isChunked = this->_headers["Transfer-Encoding"] == "chunked";
		}
		catch (const std::exception& e1) {
			throw(RequestException({"invalid Content-Length"}, 400));
		}
		if ((limitBodySize > 0) and (bodySize > limitBodySize))
			throw(RequestException({"Content-Length is longer than the maximum allowed"}, 413));
		else if ((bodySize == 0) and (isChunked == false))
			throw(RequestException({"missing or invalid Content-Length header"}, 400));
	}
	catch (const std::out_of_range& e) {
		try {
			this->_headers.at("Content-Length");
			throw(RequestException({"missing Content-Type header"}, 400));
		}
		catch (const std::out_of_range& e2) {	// no body headers, i.e. no body
			return;
		}
	}
	if (isChunked == true)
		readChunkedBody(limitBodySize);
	else
		readPlainBody(bodySize);
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

	std::string currentHost;

	try {
		currentHost = this->_headers.at("Host");
	}
	catch(std::out_of_range const& e) {
		throw(RequestException({"no Host header"}, 400));
	}
	if (this->_url.host == "")
		_setHostPort(currentHost);
	else if (this->_url.host != currentHost)
		throw(RequestException({"hosts do not match"}, 400));
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
		throw(RequestException({"unsupported HTTP version:", strVersion}, 400));
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

void	HTTPrequest::_unchunkBody( std::string const& chunkedBody)
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
