/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPstruct.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:27:03 by fra           #+#    #+#                 */
/*   Updated: 2024/03/18 05:57:04 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPstruct.hpp"

HTTPstruct::HTTPstruct( int socket, HTTPtype type ) : _socket(socket) , _type(type)
{
	this->_version.scheme = HTTP_DEF_SCHEME;
	this->_version.major = 1;
	this->_version.minor = 1;
}

bool	HTTPstruct::hasBody( void ) const noexcept
{
	return((this->_type == HTTP_CHUNKED) or (this->_type == HTTP_FILE_UPL_CGI));
}

HTTPtype	HTTPstruct::getType( void ) const noexcept
{
	return(this->_type);
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
	return (this->_type > HTTP_CHUNKED);		// NB: is chunked CGI?
}

bool	HTTPstruct::isFastCGI( void ) const noexcept
{
	return (this->_type == HTTP_FAST_CGI);
}

bool	HTTPstruct::isFileUpload( void ) const noexcept
{
	return (this->_type == HTTP_FILE_UPL_CGI);
}

bool	HTTPstruct::isChunked( void ) const noexcept
{
	return (this->_type == HTTP_CHUNKED);
}

bool	HTTPstruct::isAutoIndex( void ) const noexcept
{
	return (this->_type == HTTP_AUTOINDEX_STATIC);
}

void	HTTPstruct::_setHeaders( std::string const& headers )
{
	size_t 		del1, del2;
	std::string key, value, tmpHeaders=headers;

	if (tmpHeaders.empty())
		return ;
	del1 = tmpHeaders.find(HTTP_DEF_NL);
	do
	{
		del2 = tmpHeaders.find(": ");
		if (del2 == std::string::npos)
			throw(HTTPexception({"invalid header format:", tmpHeaders.substr(0, del1)}, 400));
		key = tmpHeaders.substr(0, del2);
		value = tmpHeaders.substr(del2 + 2, del1 - del2 - 2);
		_addHeader(key, value);
		tmpHeaders = tmpHeaders.substr(del1 + 2);
		del1 = tmpHeaders.find(HTTP_DEF_NL);
	} while (del1 != std::string::npos);
}

void	HTTPstruct::_setBody( std::string const& strBody )
{
    this->_body = strBody;
}

void	HTTPstruct::_resetTimeout( void ) noexcept
{
	this->_lastActivity = steady_clock::now();
}

void	HTTPstruct::_checkTimeout( void )
{
	duration<double> 	time_span;

	time_span = duration_cast<duration<int>>(steady_clock::now() - this->_lastActivity);
	if (time_span.count() > HTTP_MAX_TIMEOUT)
		throw(RequestException({"timeout request"}, 408));
	_resetTimeout();
}

void	HTTPstruct::_addHeader(std::string const& name, std::string const& content) noexcept
{
	this->_headers[name] = content;
}
