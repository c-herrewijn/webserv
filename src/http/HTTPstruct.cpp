/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPstruct.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:27:03 by fra           #+#    #+#                 */
/*   Updated: 2024/02/16 11:03:15 by faru          ########   odam.nl         */
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

HTTPstruct::HTTPstruct( void ) : _ready(false) , _socket(-1) {}

void	HTTPstruct::parseHead( std::string const& strReq )
{
	std::string head, headers;
	size_t		delimiter;

	delimiter = strReq.find(HTTP_TERM);
	if (delimiter == std::string::npos)
		throw(HTTPexception({"no header terminator"}, 400));
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

bool	HTTPstruct::isReady( void ) const noexcept
{
	return(this->_ready);
}

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
		this->_headers.insert({key, value});
		tmpHeaders = tmpHeaders.substr(del1 + 2);
		del1 = tmpHeaders.find(HTTP_NL);
	} while (del1 != std::string::npos);

}

void	HTTPstruct::_setBody( std::string const& strBody )
{
    this->_body = strBody;
}