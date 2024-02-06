/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:47:41 by fra           #+#    #+#                 */
/*   Updated: 2024/02/06 09:28:50 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPparser.hpp"


void	testReqs( void )
{
	int i=1;
	HTTPrequest request;
	std::vector<const char*>	reqs({
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://test:21/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://test:21/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:81\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://test/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:81\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://test:/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:81\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://test:21/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nkey1: value1\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 12much body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nContent-Type: text/plain\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo:123/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nContent-Type: text/plain\r\nContent-Length: 19\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
		// // Chunked
		// "GET http://halo:123/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nContent-Type: text/plain\r\nContent-Length: 19\r\nkey2: value2\r\n\r\n7\r\nMozilla\r\n11\r\nDeveloper Network\r\n0\r\n\r\n",
		// "GET http://halo:123/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nContent-Type: text/plain\r\nkey2: value2\r\n\r\n7\r\nMozilla\r\n11\r\nDeveloper Network\r\n0\r\n\r\n",
		// "GET http://halo:123/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nContent-Type: text/plain\r\nTransfer-Encoding: chunked\r\nkey2: value2\r\n\r\n8\r\nMozilla \r\n12\r\nDeveloper Network \r\n0\r\n\r\n",
		// hosts
		"GET http:/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\n\r\n",
		"GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\n\r\n",
		"GET http:///find/me/here?amd=123&def=566 HTTP/1.1\r\n\r\n",
		"GET http://hostname/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\n\r\n"
	});
	std::cout << "===========================================================================================\n";
	for (auto req : reqs)
	{
		try
		{
			std::cout << i++ << ". " << req << "---------\n";
			request = HTTPparser::parseRequest(req);
			std::cout << request.toString();
		}
		catch(const ParserException& e)
		{
			std::cerr << e.what() << '\n' << req;
		}
		std::cout << "===========================================================================================" << '\n';
	}
}

HTTPrequest	HTTPparser::parseRequest( std::string const& strReq )
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

void	HTTPparser::_setHead( std::string const& header, HTTPrequest &req )
{
	std::istringstream	stream(header);
	std::string 		method, url, version;

	if (! std::getline(stream, method, ' '))
		throw(ParserException({"invalid request - invalid header:", header}));
	req.head.method = strToMeth(method);
	if (! std::getline(stream, url, ' '))
		throw(ParserException({"invalid request - invalid header:", header}));
	_setURL(url, req.head.url);
	if (! std::getline(stream, version, ' '))
		throw(ParserException({"invalid request - invalid header:", header}));
	_setVersion(version, req.head.version);
	if (version.substr(version.size() - 2) != HTTP_NL)
		throw(ParserException({"invalid request - no termination header:", header}));
}

void	HTTPparser::_setHeaders( std::string const& headers, HTTPrequest &req )
{
	size_t 		del1, del2;
	std::string key, value, tmpHeaders=headers;
	bool		hostFound = false;

	if (tmpHeaders.empty())
		return ;
	del1 = tmpHeaders.find(HTTP_NL);
	do
	{
		del2 = tmpHeaders.find(": ");
		if (del2 == std::string::npos)
			throw(ParserException({"invalid request - invalid header format:", tmpHeaders}));
		key = tmpHeaders.substr(0, del2);
		if ((hostFound == false) and (key == "Host"))
			hostFound = true;
		value = tmpHeaders.substr(del2 + 2, del1 - del2 - 2);
		req.headers.insert({key, value});
		tmpHeaders = tmpHeaders.substr(del1 + 2);
		del1 = tmpHeaders.find(HTTP_NL);
	} while (del1 != std::string::npos);
	if (hostFound == false)
		throw(ParserException({"invalid request: no Host header"}));
	if (req.head.url.host == "")
		_setHostPort(req.headers["Host"], req.head.url);
	else if (req.head.url.host != req.headers["Host"])
		throw(ParserException({"invalid request: hosts do not match"}));
}

void	HTTPparser::_setBody( std::string const& body, HTTPrequest &req )
{
	bool	isChunked = false;

	if (body.empty())
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
	if (body.find(HTTP_TERM) == std::string::npos)
		throw(ParserException({"invalid request: no body terminator"}));
	if (isChunked)
		_setChunkedBody(body, req.body);
	else
		_setPlainBody(body, req);
}

void	HTTPparser::_setURL( std::string const& strURL, HTTPurl& url )
{
	size_t		delimiter;
	std::string	tmpURL=strURL;

	delimiter = tmpURL.find(":");
	if (delimiter != std::string::npos)
	{
		_setScheme(tmpURL.substr(0, delimiter), url.scheme);
		tmpURL = tmpURL.substr(delimiter + 1);
	}
	else
		_setScheme(HTTP_SCHEME, url.scheme);
	delimiter = tmpURL.find("//");
	if (delimiter != std::string::npos)
	{
		if (delimiter != 0)
			throw(ParserException({"invalid request - bad format URL:", tmpURL}));
		tmpURL = tmpURL.substr(2);
		delimiter = tmpURL.find("/");
		if (delimiter == 0)
			_setHostPort("localhost", url);
		else
			_setHostPort(tmpURL.substr(0, delimiter), url);
		tmpURL = tmpURL.substr(delimiter);
	}
	_setPath(tmpURL, url);
}

void	HTTPparser::_setScheme( std::string const& strScheme, std::string& scheme )
{
	if ((strScheme != HTTP_SCHEME) and (strScheme != HTTPS_SCHEME))
		throw(ParserException({"invalid request - unsupported scheme:", strScheme}));
	scheme = strScheme;
}

void	HTTPparser::_setHostPort( std::string const& strURL, HTTPurl& url )
{
	size_t 		delimiter = strURL.find(':');
	std::string	port;

	url.host = strURL.substr(0, delimiter);
	if (delimiter != std::string::npos)	// there's the port
	{
		port = strURL.substr(delimiter + 1);
		if (port.empty())		// because    http://ABC.com:/%7esmith/home.html is still valid
		{
			url.port = HTTP_DEF_PORT;
			return ;
		}
		try {
			url.port = std::stoi(port);
		}
		catch(const std::exception& e ) {
			throw(ParserException({"invalid request - invalid port format:", strURL.substr(delimiter + 1)}));
		}
	}
	else
		url.port = HTTP_DEF_PORT;
}

void	HTTPparser::_setPath( std::string const& strPath, HTTPurl& url )
{
	size_t		del1, del2;
	std::string	tmpPath=strPath;

	del1 = tmpPath.find('?');
	del2 = tmpPath.find('#');
	url.path = tmpPath.substr(0, std::min(del1, del2));
	if (del1 != std::string::npos)
	{
		tmpPath = tmpPath.substr(del1);
		_setQuery(tmpPath, url);
	}
	if (del2 != std::string::npos)
	{
		tmpPath = tmpPath.substr(del2);
		_setFragment(tmpPath, url.fragment);
	}
}

void	HTTPparser::_setQuery( std::string const& queries, HTTPurl& url )
{
	std::string			key, value, keyValue=queries;
	size_t 				del1, del2;

	if (queries == "?")
		throw(ParserException({"empty query"}));
	url.queryRaw = keyValue.substr(1);
	while (true)
	{
		keyValue = keyValue.substr(1);	// remove leading '?' or '&'
		del1 = keyValue.find('=');
		if (del1 == std::string::npos)
			throw(ParserException({"invalid request - invalid query:", keyValue}));
		del2 = keyValue.find('&');
		key = keyValue.substr(0, del1);
		value = keyValue.substr(del1 + 1, del2 - del1 - 1);
		if (key.empty() or value.empty())
			throw(ParserException({"invalid request - invalid query:", keyValue}));
		url.query.insert({key, value});
		if (del2 == std::string::npos)
			break;
		keyValue = keyValue.substr(del2);
	}
}

void	HTTPparser::_setFragment( std::string const& strFragment, std::string& fragment)
{
	fragment = strFragment.substr(1);
}

void	HTTPparser::_setVersion( std::string const& strVersion, HTTPversion& version )
{
	size_t	del1, del2;

	del1 = strVersion.find('/');
	if (del1 == std::string::npos)
		throw(ParserException({"invalid request - invalid version:", strVersion}));
	version.scheme = strVersion.substr(0, del1);
	std::transform(version.scheme.begin(), version.scheme.end(), version.scheme.begin(), ::tolower);
	if (version.scheme != HTTP_SCHEME)
		throw(ParserException({"invalid request - invalid scheme:", strVersion}));
	std::transform(version.scheme.begin(), version.scheme.end(), version.scheme.begin(), ::toupper);
	del2 = strVersion.find('.');
	if (del2 == std::string::npos)
		throw(ParserException({"invalid request - invalid version:", strVersion}));
	try {
		version.major = std::stoi(strVersion.substr(del1 + 1, del2 - del1 - 1));
		version.minor = std::stoi(strVersion.substr(del2 + 1));
	}
	catch (std::exception const& e) {
		throw(ParserException({"invalid request - invalid version numbers:", strVersion}));
	}
	if (version.major + version.minor != 2)
		throw(ParserException({"invalid request - unsupported HTTP version:", strVersion}));
}

void	HTTPparser::_setChunkedBody( std::string const& chunkedBody, std::string& body )
{
	size_t		sizeChunk=0, delimiter=0;
	std::string	tmpChunkedBody=chunkedBody;

	do
	{
		delimiter = tmpChunkedBody.find(HTTP_NL);
		if (delimiter == std::string::npos)
			throw(ParserException({"invalid request: bad chunking"}));
		try {
			sizeChunk = std::stoul(tmpChunkedBody.substr(0, delimiter), nullptr, 16);
		}
		catch(const std::exception& e){
			throw(ParserException({"invalid request: bad chunking"}));
		}
		body += tmpChunkedBody.substr(delimiter + 2, sizeChunk);
		tmpChunkedBody = tmpChunkedBody.substr(delimiter + sizeChunk + 4);
	} while (sizeChunk != 0);
}

void	HTTPparser::_setPlainBody( std::string const& strBody, HTTPrequest& req )
{
	req.body = strBody.substr(0, strBody.find(HTTP_TERM));
	try {
		if (req.body.size() != std::stoul(req.headers["Content-Length"]))
			throw(ParserException({"invalid request: body lengths do not match"}));
	}
	catch(const std::invalid_argument& e ) {
		throw(ParserException({"invalid Content-Length:", req.headers["Content-Length"]}));
	}
	catch(const std::out_of_range& e ) {
		throw(ParserException({"invalid Content-Length:", req.headers["Content-Length"]}));
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
