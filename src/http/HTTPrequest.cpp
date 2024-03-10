/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:40:04 by fra           #+#    #+#                 */
/*   Updated: 2024/03/09 20:43:15 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPrequest.hpp"
#include "CGI.hpp"

int	HTTPrequest::parseMain( void ) noexcept
{
	std::string	strHead, strHeaders, strBody;

	try {
		_parseHeads(strHead, strHeaders, strBody);
		_setHead(strHead);
		_setHeaders(strHeaders);
	}
	catch(const HTTPexception& e) {
		std::cout << e.what() << '\n';
		return (e.getStatus());
	}
	catch(const std::exception& e) {
		std::cout << e.what() << '\n';
		return (500);
	}
	if (strBody.empty() == false)
		this->_tmpBody = strBody;
	return (200);
}

void	HTTPrequest::_parseHeads( std::string& strHead, std::string& strHeaders, std::string& strBody )
{
	char		buffer[DEF_BUF_SIZE];
	std::string content;
	size_t		endHead=0, endReq=0;
	ssize_t		charsRead = -1;

	bzero(buffer, DEF_BUF_SIZE);
	charsRead = recv(this->_socket, buffer, DEF_BUF_SIZE, 0);
	if (charsRead < 0 )
		throw(RequestException({"unavailable socket"}, 500));
	else if (charsRead == 0)
		throw(ServerException({"connection closed"}));
	content = std::string(buffer, buffer + charsRead);
	endReq = content.find(HTTP_TERM);		// look for head + headers
	if (endReq == std::string::npos)
		throw(RequestException({"no headers terminator in request"}, 400));
	endHead = content.find(HTTP_NL);		// look for headers
	if (endHead >= endReq)
		throw(RequestException({"no headers in request"}, 400));
	strHead = content.substr(0, endHead);
	strHeaders = content.substr(endHead + HTTP_NL.size(), endReq - endHead - 1) + HTTP_NL;
	endReq += HTTP_TERM.size();
	if ((endReq + 1) < content.size())		// if there's a piece of the body
		strBody = content.substr(endReq);
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

int		HTTPrequest::validateRequest( ConfigServer const& configServer ) noexcept
{
	int statusCode = 200;
		
	this->_validator.setConfig(configServer);
	// this->_validator.setMethod(this->_method);
	// this->_validator.setPath(this->_url.path);
	// this->_validator.solvePath();
	// statusCode = this->_validator.getStatusCode();
	// if (statusCode >= 400)
	// 	return (statusCode);
	// this->_realPath = this->_validator.getRealPath();
	// this->_maxBodySize = this->_validator.getMaxBodySize();
	// try {
	// 	_checkMaxBodySize();
	// }
	// catch(const RequestException& e) {
	// 	statusCode = e.getStatus();
	// }
	return (statusCode);
}

bool	HTTPrequest::isCGI( void ) const noexcept
{
	// return (this->_validator.isCGI());
	return (this->_url.path.extension() == ".cgi");
}

bool	HTTPrequest::isAutoIndex( void ) const noexcept
{
	return (this->_validator.isAutoIndex());
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

t_path		HTTPrequest::getPath( void ) const noexcept
{
	return (this->_url.path);
}

std::string const&		HTTPrequest::getHost( void ) const noexcept
{
	return (this->_url.host);
}

std::string		HTTPrequest::getPort( void ) const noexcept
{
	return (std::to_string(this->_url.port));
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

t_path	HTTPrequest::getRealPath( void ) const noexcept
{
	if (this->_url.path == "/")		// NB: should be done by validation
		return (MAIN_PAGE_PATH);
	else if (this->_url.path.extension() == ".ico")	// NB: should be done by validation, update content-type of response
		return (FAVICON_PATH);
	else if (this->_url.path.extension() == ".cgi")
		return (t_path("/home/fra/Codam/webserv/var/www") / this->_url.path);
	else
		return (this->_url.path);
	// return (this->_realPath):
}

void	HTTPrequest::_setHead( std::string const& header )
{
	std::istringstream	stream(header);
	std::string 		method, url, version;

	if (! std::getline(stream, method, HTTP_SP))
		throw(RequestException({"invalid header:", header}, 400));
	_setMethod(method);
	if (! std::getline(stream, url, HTTP_SP))
		throw(RequestException({"invalid header:", header}, 400));
	_setURL(url);
	if (! std::getline(stream, version, HTTP_SP))
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
	this->_validator.setMethod(this->_method);
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
	size_t		queryPos, fragmentPos;
	std::string	tmpPath=strPath;

	queryPos = tmpPath.find('?');
	fragmentPos = tmpPath.find('#');
	this->_url.path = tmpPath.substr(0, std::min(queryPos, fragmentPos));
	if (queryPos != std::string::npos)
	{
		tmpPath = tmpPath.substr(queryPos);
		_setQuery(tmpPath);
	}
	if (fragmentPos != std::string::npos)
	{
		tmpPath = tmpPath.substr(fragmentPos);
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

void	HTTPrequest::_checkMaxBodySize( void )
{
	if ((hasBody() == false) or (this->_maxBodySize == 0))
		return;
	if (isChunked() == true)
	{
		if (this->_maxBodySize < this->_tmpBody.size())
			throw(RequestException({"Content-Length longer than config max body length"}, 413));
	}
	else
	{
		if (this->_maxBodySize < this->_contentLength)
			throw(RequestException({"Content-Length longer than config max body length"}, 413));
	}
}

void	HTTPrequest::_setHeaders( std::string const& strHeaders )
{
	size_t	contentLength = 0;
	HTTPstruct::_setHeaders(strHeaders);

	if (this->_headers.count("Host") == 0)
		throw(RequestException({"no Host header"}, 412));
	if (this->_url.host == "")
		_setHostPort(this->_headers["Host"]);
	else if (this->_headers["Host"].find(this->_url.host) == std::string::npos)
		throw(RequestException({"hosts do not match"}, 412));
	if (this->_headers.count("Connecton") != 0)
		this->_endConn = this->_headers["Connection"] == "close";
	if (this->_headers.count("Content-Length") == 0)
	{
		if (this->_headers.count("Transfer-Encoding") == 0)		// no body
			return ;
		else if (this->_headers.at("Transfer-Encoding") == "chunked")
			this->_isChunked = true;
	}
	else
	{
		try {
			contentLength = std::stoull(this->_headers["Content-Length"]);
		}
		catch (const std::exception& e) {
			throw(RequestException({"invalid Content-Length"}, 400));
		}
		if (this->_tmpBody.size() > contentLength)
			throw(RequestException({"body is longer than Content-Length"}, 400));
		if (this->_headers.count("Content-Type") != 0)
		{
			if (this->_headers["Content-Type"].find("multipart/form-data; boundary=-") == 0)
				this->_isFileUpload = true;
		}
	}
	this->_contentLength = contentLength;
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

// std::string	HTTPrequest::_unchunkBody( std::string const& chunkedBody)
// {
// 	size_t		sizeChunk=0, delimiter=0;
// 	std::string	tmpChunkedBody = chunkedBody;

// 	if (chunkedBody.find(HTTP_TERM) == std::string::npos)
// 		throw(RequestException({"no body terminator"}, 400));
// 	do
// 	{
// 		delimiter = tmpChunkedBody.find(HTTP_NL);
// 		if (delimiter == std::string::npos)
// 			throw(RequestException({"bad chunking"}, 400));
// 		try {
// 			sizeChunk = std::stoul(tmpChunkedBody.substr(0, delimiter), nullptr, 16);
// 			this->_body += tmpChunkedBody.substr(delimiter + HTTP_NL.size(), sizeChunk);
// 			tmpChunkedBody = tmpChunkedBody.substr(delimiter + sizeChunk + HTTP_NL.size() * 2);
// 		}
// 		catch(const std::exception& e){
// 			throw(RequestException({"bad chunking"}, 400));
// 		}
// 	} while (sizeChunk != 0);
// 	return (tmpChunkedBody);
// }

// std::string	HTTPrequest::_unchunkChunk( std::string const& chunkedChunk, std::string& remainder)
// {
// 	size_t		sizeChunk=0, delimiter=0;
// 	std::string	tmpChunkedBody = chunkedChunk;
// 	std::string	unchunkedChunks;

// 	while (true)
// 	{
// 		delimiter = tmpChunkedBody.find(HTTP_NL);
// 		if (delimiter == std::string::npos)
// 			throw(RequestException({"bad chunking"}, 400));
// 		try {
// 			sizeChunk = std::stoul(tmpChunkedBody.substr(0, delimiter), nullptr, 16);
// 			if ((delimiter + sizeChunk + HTTP_NL.size() * 2) < tmpChunkedBody.size())
// 			{
// 				remainder = tmpChunkedBody;
// 				break;
// 			}
// 		}
// 		catch(const std::exception& e){
// 			throw(RequestException({"bad chunking"}, 400));
// 		}
// 	}
// 	return (unchunkedChunks);
// }

// void	HTTPrequest::readPlainBody( void )
// {
//     ssize_t 			readChar = -1;
//     char        		buffer[DEF_BUF_SIZE + 1];
// 	static std::string	body = this->_tmpBody;
// 	static size_t		countChars = this->_tmpBody.length();

// 	static steady_clock::time_point 	lastRead = steady_clock::now();
// 	steady_clock::time_point 			currentRead;
// 	duration<double> 					time_span;

// 	if (this->_gotFullBody)
// 		throw(ServerException({"body already parsed"}));
// 	else if (this->_socket == -1)
// 		throw(RequestException({"invalid socket"}, 500));
// 	ft_bzero(buffer, DEF_BUF_SIZE + 1);
// 	readChar = recv(this->_socket, buffer, DEF_BUF_SIZE, 0);
// 	if (readChar < 0 )
// 		throw(RequestException({"unavailable socket"}, 500));
// 	else if (readChar == 0)
// 	{
// 		currentRead = steady_clock::now();
// 		time_span = duration_cast<duration<int>>(currentRead - lastRead);
// 		if (time_span.count() > MAX_TIMEOUT)
// 			throw(RequestException({"timeout request"}, 408));
// 	}
// 	else
// 	{
// 		lastRead = steady_clock::now();
// 		countChars += readChar;
// 		if (countChars > this->_contentLength)
// 			throw(RequestException({"content body is longer than expected"}, 413));
// 		body += buffer;
// 		// write buffer to CGI

// 		if (countChars == this->_contentLength)
// 			this->_gotFullBody = true;
// 	}
// }

// void	HTTPrequest::readChunkedBody( void )
// {
//     ssize_t 	readChar = -1;
//     char    	buffer[DEF_BUF_SIZE + 1];
// 	static std::string body = this->_tmpBody;
// 	size_t		countChars=this->_tmpBody.length();

// 	static steady_clock::time_point 	lastRead = steady_clock::now();
// 	steady_clock::time_point 			currentRead;
// 	duration<double> 					time_span;

// 	if (this->_socket == -1)
// 		throw(RequestException({"invalid socket"}, 500));
// 	ft_bzero(buffer, DEF_BUF_SIZE + 1);
// 	readChar = recv(this->_socket, buffer, DEF_BUF_SIZE, 0);
// 	if (readChar < 0 )
// 		throw(RequestException({"unavailable socket"}, 500));
// 	else if (readChar == 0)
// 	{
// 		currentRead = steady_clock::now();
// 		time_span = duration_cast<duration<int>>(currentRead - lastRead);
// 		if (time_span.count() > MAX_TIMEOUT)
// 			throw(RequestException({"timeout request"}, 408));
// 	}
// 	else
// 	{
// 		lastRead = steady_clock::now();
// 		countChars += readChar;
// 		if (countChars > this->_contentLength)
// 			throw(RequestException({"content body is longer than the maximum allowed"}, 413));
// 		body += buffer;
// 		if (std::string(buffer).find(HTTP_TERM) != std::string::npos)
// 		{
// 			this->_gotFullBody = true;
// 			body = _unchunkBody(body);
// 		}
// 	}
// 	// write unchunked body to CGI?
// 	// HTTPstruct::_setBody(body);
// }
