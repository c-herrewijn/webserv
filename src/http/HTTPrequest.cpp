/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:40:04 by fra           #+#    #+#                 */
/*   Updated: 2024/02/11 03:22:52 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPrequest.hpp"

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

HTTPrequest::HTTPrequest( void ) : HTTPstruct() {}

void	HTTPrequest::setTmpBody( std::string const& tmpBody)
{
	this->_tmpBody = tmpBody;
}

std::string const&		HTTPrequest::getTmpBody( void ) const
{
	return (this->_tmpBody);
}

void	HTTPrequest::parseBody( std::string const& strBody)
{
	bool 		isChunked = false;
	std::string fullBody = this->_tmpBody + strBody;

    if (fullBody.empty() == false)
	{
		try {
			this->_headers.at("Content-Type");
			try {
				this->_headers.at("Content-Length");
			}
			catch (const std::out_of_range& e) {
				if (this->_headers["Transfer-Encoding"] == "chunked")
					isChunked = true;
				else
					throw(RequestException({"no Content-Length header"}));
			}
		}
		catch(const std::out_of_range& e) {
			throw(RequestException({"no Content-Type header"}));
		}
		if (isChunked == true)
			_setChunkedBody(fullBody);
		else
			_setBody(fullBody);
	}
	this->_ready = true;
}

std::string	HTTPrequest::toString( void ) const
{
	std::string	strReq;

	switch (this->_method)
	{
		case HTTP_GET:
		{
			strReq += "GET";
			break;
		}
		case HTTP_POST:
		{
			strReq += "POST";
			break;
		}
		case HTTP_DELETE:
		{
			strReq += "DELETE";
			break;
		}
		default:
			throw(RequestException({"Unknown HTTP method"}));
	}
	strReq += HTTP_SP;
	strReq += this->_url.toString();
	strReq += HTTP_SP;
	strReq += this->_version.toString();
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
	{
		strReq += this->_body;
		strReq += HTTP_TERM;
	}
	return (strReq);
}

std::string	HTTPrequest::getHost( void ) const
{
	std::string hostStr;
	size_t		delim;

	try {
		hostStr = this->_headers.at("Host");
	}
	catch(const std::out_of_range& e) {
		throw(RequestException({"no host found in request"}));
	}
	delim = hostStr.find(':');
	return (hostStr.substr(0, delim));
}

void	HTTPrequest::_setHead( std::string const& header )
{
	std::istringstream	stream(header);
	std::string 		method, url, version;

	if (! std::getline(stream, method, ' '))
		throw(RequestException({"invalid header:", header}));
	_setMethod(method);
	if (! std::getline(stream, url, ' '))
		throw(RequestException({"invalid header:", header}));
	_setURL(url);
	if (! std::getline(stream, version, ' '))
		throw(RequestException({"invalid header:", header}));
	_setVersion(version);
	if (version.substr(version.size() - 2) != HTTP_NL)
		throw(RequestException({"no termination header:", header}));
}

void	HTTPrequest::_setHeaders( std::string const& headers)
{
	HTTPstruct::_setHeaders(headers);

	try {
		std::string currentHost = this->_headers.at("Host");
		if (this->_url.host == "")
			_setHostPort(currentHost);
		else if (this->_url.host != currentHost)
		{
			std::cout << this->_url.host << " - " << currentHost << '\n';
			throw(RequestException({"hosts do not match"}));
		}
	}
	catch(std::out_of_range const& e) {
		throw(RequestException({"no Host header"}));
	}	
}

void	HTTPrequest::_setBody( std::string const& strBody )
{
    std::string tmpBody;
    size_t      delimiter = strBody.find(HTTP_TERM);

    if (delimiter == std::string::npos)
		throw(RequestException({"no body terminator"}));
	tmpBody = strBody.substr(0, delimiter);
	try {
		if (tmpBody.size() != std::stoul(this->_headers["Content-Length"]))
			throw(RequestException({"body lengths do not match"}));
	}
	catch(const std::invalid_argument& e ) {
		throw(RequestException({"invalid Content-Length:", this->_headers["Content-Length"]}));
	}
	catch(const std::out_of_range& e ) {
		throw(RequestException({"missing or overflow Content-Length header"}));
	}
	HTTPstruct::_setBody(tmpBody);
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
		throw(RequestException({"unknown HTTP method:", strMethod}));
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
			throw(RequestException({"bad format URL:", tmpURL}));
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
		throw(RequestException({"invalid version:", strVersion}));
	this->_version.scheme = strVersion.substr(0, del1);
	std::transform(this->_version.scheme.begin(), this->_version.scheme.end(), this->_version.scheme.begin(), ::tolower);
	if (this->_version.scheme != HTTP_SCHEME)
		throw(RequestException({"invalid scheme:", strVersion}));
	std::transform(this->_version.scheme.begin(), this->_version.scheme.end(), this->_version.scheme.begin(), ::toupper);
	del2 = strVersion.find('.');
	if (del2 == std::string::npos)
		throw(RequestException({"invalid version:", strVersion}));
	try {
		this->_version.major = std::stoi(strVersion.substr(del1 + 1, del2 - del1 - 1));
		this->_version.minor = std::stoi(strVersion.substr(del2 + 1));
	}
	catch (std::exception const& e) {
		throw(RequestException({"invalid version numbers:", strVersion}));
	}
	if (this->_version.major + this->_version.minor != 2)
		throw(RequestException({"unsupported HTTP version:", strVersion}));
}

void	HTTPrequest::_setScheme( std::string const& strScheme )
{
	if ((strScheme != HTTP_SCHEME) and (strScheme != HTTPS_SCHEME))
		throw(RequestException({"unsupported scheme:", strScheme}));
	this->_url.scheme = strScheme;
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
			throw(RequestException({"invalid port format:", strURL.substr(delimiter + 1)}));
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
		throw(RequestException({"empty query"}));
	this->_url.queryRaw = keyValue.substr(1);
	while (true)
	{
		keyValue = keyValue.substr(1);	// remove leading '?' or '&'
		del1 = keyValue.find('=');
		if (del1 == std::string::npos)
			throw(RequestException({"invalid query:", keyValue}));
		del2 = keyValue.find('&');
		key = keyValue.substr(0, del1);
		value = keyValue.substr(del1 + 1, del2 - del1 - 1);
		if (key.empty() or value.empty())
			throw(RequestException({"invalid query:", keyValue}));
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
	std::string	tmpChunkedBody=chunkedBody;

	if (chunkedBody.find(HTTP_TERM) == std::string::npos)
		throw(RequestException({"no body terminator"}));
	do
	{
		delimiter = tmpChunkedBody.find(HTTP_NL);
		if (delimiter == std::string::npos)
			throw(RequestException({"bad chunking"}));
		try {
			sizeChunk = std::stoul(tmpChunkedBody.substr(0, delimiter), nullptr, 16);
			this->_body += tmpChunkedBody.substr(delimiter + HTTP_NL.size(), sizeChunk);
			tmpChunkedBody = tmpChunkedBody.substr(delimiter + sizeChunk + HTTP_NL.size() * 2);
		}
		catch(const std::exception& e){
			throw(RequestException({"bad chunking"}));
		}
	} while (sizeChunk != 0);
}