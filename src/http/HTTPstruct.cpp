/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPstruct.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:27:03 by fra           #+#    #+#                 */
/*   Updated: 2024/02/18 03:20:50 by fra           ########   odam.nl         */
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

HTTPstruct::HTTPstruct( void ) : _socket(-1) {}

void	HTTPstruct::setSocket( int socket )
{
	if (socket == -1)
		throw(HTTPexception({"invalid socket"}, 500));
	this->_socket = socket;
}

int		HTTPstruct::getSocket( void ) const noexcept
{
	return (this->_socket);
}

void	HTTPstruct::_setHeaders( std::string const& headers )
{
	size_t 		del1, del2;
	std::string key, value, tmpHeaders=headers;

	if (tmpHeaders.empty())
		return ;
	del1 = tmpHeaders.find(HTTP_NL);
	do
	{
		del2 = tmpHeaders.find(": ");
		if (del2 == std::string::npos)
			throw(HTTPexception({"invalid header format:", tmpHeaders}, 400));
		key = tmpHeaders.substr(0, del2);
		value = tmpHeaders.substr(del2 + 2, del1 - del2 - 2);
		_addHeader(key, value);
		tmpHeaders = tmpHeaders.substr(del1 + 2);
		del1 = tmpHeaders.find(HTTP_NL);
	} while (del1 != std::string::npos);
}

void	HTTPstruct::_addHeader(std::string const& name, std::string const& content) noexcept
{
	this->_headers[name] = content;
}

void	HTTPstruct::_setBody( std::string const& strBody )
{
    this->_body = strBody;
}
