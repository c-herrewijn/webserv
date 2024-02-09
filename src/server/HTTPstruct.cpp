/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPstruct.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:27:03 by fra           #+#    #+#                 */
/*   Updated: 2024/02/09 17:46:54 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPstruct.hpp"

// void	testReqs( void )
// {
// 	int i=1;
// 	HTTPrequest request;
// 	std::vector<const char*>	reqs({
// 		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nmuch body very http\r\n\r\n",
// 		// "GET http://test:21/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
// 		// "GET http://test:21/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:81\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
// 		// "GET http://test/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:81\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
// 		// "GET http://test:/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:81\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
// 		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
// 		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
// 		// "GET http://test:21/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nkey1: value1\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
// 		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
// 		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 12much body very http\r\n\r\n",
// 		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n",
// 		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\n",
// 		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
// 		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\n",
// 		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
// 		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nContent-Type: text/plain\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
// 		// "GET http://halo:123/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nContent-Type: text/plain\r\nContent-Length: 19\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
// 		// // Chunked
// 		// "GET http://halo:123/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nContent-Type: text/plain\r\nContent-Length: 19\r\nkey2: value2\r\n\r\n7\r\nMozilla\r\n11\r\nDeveloper Network\r\n0\r\n\r\n",
// 		// "GET http://halo:123/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nContent-Type: text/plain\r\nkey2: value2\r\n\r\n7\r\nMozilla\r\n11\r\nDeveloper Network\r\n0\r\n\r\n",
// 		// "GET http://halo:123/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nContent-Type: text/plain\r\nTransfer-Encoding: chunked\r\nkey2: value2\r\n\r\n8\r\nMozilla \r\n12\r\nDeveloper Network \r\n0\r\n\r\n",
// 		// hosts
// 		"GET http:/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\n\r\n",
// 		"GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\n\r\n",
// 		"GET http:///find/me/here?amd=123&def=566 HTTP/1.1\r\n\r\n",
// 		"GET http://hostname/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\n\r\n"
// 	});
// 	std::cout << "===========================================================================================\n";
// 	for (auto req : reqs)
// 	{
// 		try
// 		{
// 			std::cout << i++ << ". " << req << "---------\n";
// 			request = HTTPparser::parseRequest(req);
// 			std::cout << request.toString();
// 		}
// 		catch(const ServerException& e)
// 		{
// 			std::cerr << e.what() << '\n' << req;
// 		}
// 		std::cout << "===========================================================================================" << '\n';
// 	}
// }

std::string	HTTPversion::toString( void ) const
{
	std::string	strVersion;
	strVersion += this->scheme;
	strVersion += "/";
	strVersion += std::to_string(this->major);
	strVersion += ".";
	strVersion += std::to_string(this->minor);
	return (strVersion);
}

void	HTTPstruct::parseHead( std::string const& strReq )
{
	std::string head, headers;
	size_t		delimiter;

	delimiter = strReq.find(HTTP_TERM);
	if (delimiter == std::string::npos)
		throw(ServerException({"invalid request: no terminator"}));
	head = strReq.substr(0, delimiter + 2);
	delimiter = head.find(HTTP_NL);
	if (delimiter + 2 != head.size())		// we have the headers
	{
		headers = head.substr(delimiter + 2);
		head = head.substr(0, delimiter + 2);
	}
	_setHead(head);
	_setHeaders(headers);
}

void	HTTPstruct::_setHeaders( std::string const& headers )
{
	size_t 		del1, del2;
	std::string key, value, tmpHeaders=headers;
	// bool		hostFound = false;

	if (tmpHeaders.empty())
		return ;
	del1 = tmpHeaders.find(HTTP_NL);
	do
	{
		del2 = tmpHeaders.find(": ");
		if (del2 == std::string::npos)
			throw(ServerException({"invalid request - invalid header format:", tmpHeaders}));
		key = tmpHeaders.substr(0, del2);
		// if ((hostFound == false) and (key == "Host"))
		// 	hostFound = true;
		value = tmpHeaders.substr(del2 + 2, del1 - del2 - 2);
		this->_headers.insert({key, value});
		tmpHeaders = tmpHeaders.substr(del1 + 2);
		del1 = tmpHeaders.find(HTTP_NL);
	} while (del1 != std::string::npos);

}

void	HTTPstruct::_setVersion( std::string const& strVersion )
{
	size_t	del1, del2;

	del1 = strVersion.find('/');
	if (del1 == std::string::npos)
		throw(ServerException({"invalid request - invalid version:", strVersion}));
	this->_version.scheme = strVersion.substr(0, del1);
	std::transform(this->_version.scheme.begin(), this->_version.scheme.end(), this->_version.scheme.begin(), ::tolower);
	if (this->_version.scheme != HTTP_SCHEME)
		throw(ServerException({"invalid request - invalid scheme:", strVersion}));
	std::transform(this->_version.scheme.begin(), this->_version.scheme.end(), this->_version.scheme.begin(), ::toupper);
	del2 = strVersion.find('.');
	if (del2 == std::string::npos)
		throw(ServerException({"invalid request - invalid version:", strVersion}));
	try {
		this->_version.major = std::stoi(strVersion.substr(del1 + 1, del2 - del1 - 1));
		this->_version.minor = std::stoi(strVersion.substr(del2 + 1));
	}
	catch (std::exception const& e) {
		throw(ServerException({"invalid request - invalid version numbers:", strVersion}));
	}
	if (this->_version.major + this->_version.minor != 2)
		throw(ServerException({"invalid request - unsupported HTTP version:", strVersion}));
}

void	HTTPstruct::_setBody( std::string const& strBody )
{
    std::string tmpBody;
    size_t      delimiter = strBody.find(HTTP_TERM);

    if (delimiter == std::string::npos)
		throw(ServerException({"invalid request: no body terminator"}));
	tmpBody = strBody.substr(0, delimiter);
	try {
		if (tmpBody.size() != std::stoul(this->_headers["Content-Length"]))
			throw(ServerException({"invalid request: body lengths do not match"}));
	}
	catch(const std::invalid_argument& e ) {
		throw(ServerException({"invalid Content-Length:", this->_headers["Content-Length"]}));
	}
	catch(const std::out_of_range& e ) {
		throw(ServerException({"missing or overflow Content-Length header"}));
	}
    this->_body = tmpBody;
}