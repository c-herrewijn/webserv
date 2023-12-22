/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:47:41 by fra           #+#    #+#                 */
/*   Updated: 2023/12/06 21:05:28 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPparser.hpp"
#include "WebServer.hpp"

#include <iostream>
#include <string.h>
#include <unistd.h>

HTTPreqStatus_t	HTTPparser::parse( int connfd, HTTPrequest_t *req )
{
	char	*eoh, *eol;
	char	buffer[HEADER_MAX_SIZE + 1];
	ssize_t readChar;

	memset(buffer, 0, HEADER_MAX_SIZE + 1);
	readChar = read(connfd, buffer, HEADER_MAX_SIZE);
	if (readChar < 0)
		throw(ServerException({"socket not available or empty"}));
	else if (readChar == 0)
		return (FMT_OK);
	eoh = strstr(buffer, "\r\n\r\n");
	if (eoh == nullptr)
		return (FMT_BIGHEAD);
	else if (eoh == buffer)
		return (FMT_BADFMT);
	eoh[0] = '\0';
	req->request = buffer;
	eol = strstr(buffer, "\r\n");
	if (eol != nullptr) // if there are options
	{
		eol[0] = '\0';
		req->hasOpts = true;
		if (_getOptions(eol + 2, req->options) == FMT_BADOPT)
			return (FMT_BADOPT);
	}
	req->body = _getBody(eoh + 4, connfd);
	req->hasBody = !req->body.empty();
	return (FMT_OK);
}

void	HTTPparser::printData( HTTPrequest_t httpReq ) noexcept
{
	std::cout << "request: " << httpReq.request << "\n";
	for(auto option : httpReq.options)
		std::cout << "\toption: " << option.first << ": " << option.second << "\n";
	std::cout << "body: " << httpReq.body << "\n";
}

const char*	HTTPparser::printStatus( HTTPreqStatus_t stat ) noexcept
{
	(void) stat;
	return ("error!");
	// mapping enum -> char*, print msg
	// overload of << operator instead of class method
}

HTTPreqStatus_t	HTTPparser::_getOptions( char *line, std::map<std::string, std::string>& options ) noexcept
{
	char *eol, *colon;

	while (true)
	{
		eol = strstr(line, "\r\n");
		if (eol == nullptr)
			break;
		eol[0] = '\0';
		colon = strchr(line, ':');
		if (colon == nullptr)
			return (FMT_BADOPT);
		*colon = '\0';
		options.insert({line, colon + 2});
		line = eol + 2;
	}
	return (FMT_OK);
}

std::string	HTTPparser::_getBody( char *startBody, int connfd )
{
	ssize_t			readChar;
	char			buffer[HEADER_MAX_SIZE + 1];
	std::string		body;

	body = startBody;
	while (true)
	{
		memset(buffer, 0, HEADER_MAX_SIZE + 1);
		readChar = read(connfd, buffer, HEADER_MAX_SIZE);
		if (readChar < 0)
			throw(ServerException({"socket not available"}));
		else if (readChar == 0)
			break ;
		buffer[readChar] = '\0';
		body += buffer;
	}
	return (body);
}

HTTPparser::~HTTPparser( void ) noexcept
{
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
