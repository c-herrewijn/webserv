/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:47:41 by fra           #+#    #+#                 */
/*   Updated: 2023/11/26 23:56:26 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPparser.hpp"

HTTPparser::~HTTPparser( void )
{
	this->_freeNodes();
}

void	HTTPparser::parse( int connfd )
{
	char	*eoh;
	char	buffer[HEADER_MAX_SIZE + 1];
	ssize_t readChar;

	if (this->_optionalHead != nullptr)
		this->_freeNodes();
	memset(buffer, 0, HEADER_MAX_SIZE + 1);
	readChar = read(connfd, buffer, HEADER_MAX_SIZE);
	if (readChar <= 0)
		throw(ServerException({"socket not available or empty"}));
	eoh = strstr(buffer, "\r\n\r\n");
	if (eoh == NULL)
		throw(ServerException({"header exceeds 8 KB maximum"}));
	else if (eoh == buffer)
		throw(ServerException({"empty header"}));
	eoh[2] = '\0';
	this->parseHeader(buffer);
	this->_body = eoh + 4;
	if (readChar == HEADER_MAX_SIZE)
		this->parseBody(connfd);
}

void	HTTPparser::parseHeader( char *buffer )
{
	char *eol, *line;

	eol = strstr(buffer, "\r\n");
	eol[0] = '\0';
	this->_httpReq = buffer;
	line = eol + 2;
	while (true)
	{
		eol = strstr(line, "\r\n");
		if (eol == NULL)
			break;
		eol[0] = '\0';
		_addNode(line);
		line = eol + 2;
	}
}

void	HTTPparser::parseBody( int connfd )
{
	ssize_t	readChar;
	char	buffer[HEADER_MAX_SIZE + 1];

	while (true)
	{
		memset(buffer, 0, HEADER_MAX_SIZE + 1);
		readChar = read(connfd, buffer, HEADER_MAX_SIZE);
		if (readChar < 0)
			throw(ServerException({"socket not available"}));
		buffer[readChar] = '\0';
		this->_body += buffer;
		if (readChar < 1)
			break ;
	}
}

void	HTTPparser::printData( void ) const
{
	std::cout << "request: " << this->_httpReq << "\n";
	for(auto tmp = this->_optionalHead; tmp != nullptr; tmp = tmp->next)
		std::cout << "\toption: " << tmp->key << ": " << tmp->content << "\n";
	std::cout << "body: " << this->_body << "\n";
}

std::string const&	HTTPparser::getRequest( void ) const
{
	return (this->_httpReq);
}

std::string const&	HTTPparser::getBody( void ) const
{
	return (this->_body);
}

std::string			HTTPparser::getHeader( void ) const
{
	std::string fullHeader = this->_httpReq;
	fullHeader += "\r\n";
	for(auto tmp = this->_optionalHead; tmp != nullptr; tmp = tmp->next)
		fullHeader += tmp->content + ": " + tmp->key + "\r\n";
	fullHeader += "\r\n";
	return (fullHeader);
}

void	HTTPparser::_addNode( char* nodeContent )
{
	char		*colon;
	HTTPlist_t	*newNode, *tmp;

	colon = strchr(nodeContent, ':');
	if (colon == nullptr)
		throw(ServerException({"bad format option header line: -", nodeContent}));
	newNode = new HTTPlist_t();
	*colon = '\0';
	newNode->key = nodeContent;
	newNode->content = colon + 2;
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

void	HTTPparser::_freeNodes( void )
{
	HTTPlist_t	*toDrop;
	while (this->_optionalHead)
	{
		toDrop = this->_optionalHead;
		this->_optionalHead = this->_optionalHead->next;
		delete toDrop;
	}
}

HTTPparser::HTTPparser( HTTPparser const& other )
{
	(void) other;
}

HTTPparser& HTTPparser::operator=( HTTPparser const& other )
{
	(void) other;
	return (*this);
}
