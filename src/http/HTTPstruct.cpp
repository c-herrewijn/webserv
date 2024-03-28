/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPstruct.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:27:03 by fra           #+#    #+#                 */
/*   Updated: 2024/03/26 01:49:29 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPstruct.hpp"

HTTPstruct::HTTPstruct( int socket, HTTPtype type ) : _socket(socket) , _type(type)
{
	this->_version.scheme = HTTP_DEF_SCHEME;
	this->_version.major = 1;
	this->_version.minor = 1;
}

HTTPtype			HTTPstruct::getType( void ) const noexcept
{
	return(this->_type);
}

int					HTTPstruct::getSocket( void ) const noexcept
{
	return (this->_socket);
}

std::string const&	HTTPstruct::getServName( void ) const noexcept
{
	return(this->_servName);
}

void				HTTPstruct::setServName( std::string nameServ) noexcept
{
	this->_servName = nameServ;
}

std::string const&	HTTPstruct::getTmpBody( void )
{
	return (this->_tmpBody);
}

void				HTTPstruct::setTmpBody( std::string const& tmpBody )
{
    this->_tmpBody = tmpBody;
}

t_path const&	HTTPstruct::getRoot( void ) const noexcept
{
	return (this->_root);
}

void	HTTPstruct::setRoot( t_path const& newRoot ) noexcept
{
	this->_root = newRoot;
}

bool	HTTPstruct::isStatic( void ) const noexcept
{
	return (this->_type == HTTP_STATIC);
}

bool	HTTPstruct::isAutoIndex( void ) const noexcept
{
	return (this->_type == HTTP_AUTOINDEX_STATIC);
}

bool	HTTPstruct::isChunked( void ) const noexcept
{
	return (this->_type == HTTP_CHUNKED);
}

bool	HTTPstruct::isFastCGI( void ) const noexcept
{
	return (this->_type == HTTP_FAST_CGI);
}

bool	HTTPstruct::isFileUpload( void ) const noexcept
{
	return (this->_type == HTTP_FILE_UPL_CGI);
}

bool	HTTPstruct::isCGI( void ) const noexcept
{
	return (this->_type > HTTP_CHUNKED);
}

void	HTTPstruct::_setHeaders( std::string const& headers )
{
	size_t 		nextHeader, delimHeader;
	std::string key, value, tmpHeaders=headers;

	if (tmpHeaders.empty())
		return ;
	nextHeader = tmpHeaders.find(HTTP_DEF_NL);
	while (nextHeader != std::string::npos)
	{
		delimHeader = tmpHeaders.find(": ");
		if (delimHeader == std::string::npos)
			throw(HTTPexception({"invalid header format:", tmpHeaders.substr(0, nextHeader)}, 400));
		key = tmpHeaders.substr(0, delimHeader);
		value = tmpHeaders.substr(delimHeader + 2, nextHeader - delimHeader - 2);
		tmpHeaders = tmpHeaders.substr(nextHeader + HTTP_DEF_NL.size());
		_addHeader(key, value);
		nextHeader = tmpHeaders.find(HTTP_DEF_NL);
	}
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
