/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:47:41 by fra           #+#    #+#                 */
/*   Updated: 2023/11/30 01:31:24 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPparser.hpp"

HTTPparser::~HTTPparser( void ) noexcept
{
	this->_freeNodes();
}

void	HTTPparser::parse( int connfd )
{
	char	*eoh;
	char	buffer[HEADER_MAX_SIZE + 1];
	ssize_t readChar;

	memset(buffer, 0, HEADER_MAX_SIZE + 1);
	readChar = read(connfd, buffer, HEADER_MAX_SIZE);
	if (readChar <= 0)
		throw(ServerException({"socket not available or empty"}));
	eoh = strstr(buffer, "\r\n\r\n");
	if (eoh == nullptr)
		throw(ServerException({"header exceeds 8 KB maximum"}));
	else if (eoh == buffer)
		throw(ServerException({"empty header"}));
	eoh[2] = '\0';
	this->parseHeader(buffer);
	this->parseBody(eoh + 4, connfd);
}

void	HTTPparser::parseHeader( char *buffer )
{
	char *eol, *line, *colon;

	eol = strstr(buffer, "\r\n");
	eol[0] = '\0';
	this->_httpReq = buffer;
	line = eol + 2;
	if (this->_optionalHead != nullptr)
		this->_freeNodes();
	while (true)
	{
		eol = strstr(line, "\r\n");
		if (eol == nullptr)
			break;
		eol[0] = '\0';
		colon = strchr(line, ':');
		if (colon == nullptr)
			throw(ServerException({"bad format option header line: -", line}));
		*colon = '\0';
		_addNode(line, colon + 2);
		line = eol + 2;
	}
}

void	HTTPparser::parseBody( char *startBody, int connfd )
{
	ssize_t	readChar;
	char	buffer[HEADER_MAX_SIZE + 1];

	this->_body = startBody;
	while (true)
	{
		memset(buffer, 0, HEADER_MAX_SIZE + 1);
		readChar = read(connfd, buffer, HEADER_MAX_SIZE);
		if (readChar < 0)
			throw(ServerException({"socket not available"}));
		else if (readChar == 0)
			break ;
		buffer[readChar] = '\0';
		this->_body += buffer;
	}
}

void	HTTPparser::printData( void ) const noexcept
{
	std::cout << "request: " << this->_httpReq << "\n";
	for(auto tmp = this->_optionalHead; tmp != nullptr; tmp = tmp->next)
		std::cout << "\toption: " << tmp->key << ": " << tmp->content << "\n";
	std::cout << "body: " << this->_body << "\n";
}

std::string const&	HTTPparser::getRequest( void ) const noexcept
{
	return (this->_httpReq);
}

std::string const&	HTTPparser::getBody( void ) const noexcept
{
	return (this->_body);
}

std::string			HTTPparser::getHeader( void ) const noexcept
{
	std::string fullHeader = this->_httpReq;
	fullHeader += "\r\n";
	for(auto tmp = this->_optionalHead; tmp != nullptr; tmp = tmp->next)
		fullHeader += tmp->content + ": " + tmp->key + "\r\n";
	fullHeader += "\r\n";
	return (fullHeader);
}

void	HTTPparser::_addNode( char *key, char *content ) noexcept
{
	HTTPlist_t	*newNode, *tmp;

	newNode = new HTTPlist_t();
	newNode->key = key;
	newNode->content = content;
	newNode->next = nullptr;
	if (this->_optionalHead == nullptr)
		this->_optionalHead = newNode;
	else
	{
		tmp = this->_optionalHead;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = newNode;
	}
}

void	HTTPparser::_freeNodes( void ) noexcept
{
	HTTPlist_t	*toDrop;
	while (this->_optionalHead)
	{
		toDrop = this->_optionalHead;
		this->_optionalHead = this->_optionalHead->next;
		delete toDrop;
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
