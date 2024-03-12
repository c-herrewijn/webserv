/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPstruct.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:27:03 by fra           #+#    #+#                 */
/*   Updated: 2024/03/12 17:48:39 by faru          ########   odam.nl         */
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

HTTPstruct::HTTPstruct( int socket ) : _hasBody(false) , _gotFullBody(false) , _isCGI(false) , _isFileUpload(false)
{
	this->_version.scheme = HTTP_SCHEME;
	this->_version.major = 1;
	this->_version.minor = 1;
	this->_socket = socket;
}

bool	HTTPstruct::hasBody( void) const noexcept
{
	return(this->_hasBody);
}

int		HTTPstruct::getSocket( void ) const noexcept
{
	return (this->_socket);
}

std::string const&	HTTPstruct::getServName( void ) const noexcept
{
	return(this->_servName);
}

void	HTTPstruct::setServName( std::string nameServ) noexcept
{
	this->_servName = nameServ;
}

std::string const&	HTTPstruct::getTmpBody( void )
{
	return (this->_tmpBody);
}

void	HTTPstruct::setTmpBody( std::string const& tmpBody )
{
    this->_tmpBody = tmpBody;
}

bool	HTTPstruct::isCGI( void ) const noexcept
{
	return (this->_isCGI);
}

void	HTTPstruct::setIsCGI( bool isCGI ) noexcept
{
	this->_isCGI = isCGI;
}

bool	HTTPstruct::isFileUpload( void ) const noexcept
{
	return (this->_isFileUpload);
}

void	HTTPstruct::setFileUpload( bool isFileUpload ) noexcept
{
	this->_isFileUpload = isFileUpload;
}

bool	HTTPstruct::gotFullBody( void ) const noexcept
{
	return(this->_gotFullBody);
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

void	HTTPstruct::_setBody( std::string const& strBody )
{
    if (strBody.empty())
		return ;
	this->_hasBody = true;
    this->_body = strBody;
}

void	HTTPstruct::_addHeader(std::string const& name, std::string const& content) noexcept
{
	this->_headers[name] = content;
}
