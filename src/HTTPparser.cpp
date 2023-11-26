/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:47:41 by fra           #+#    #+#                 */
/*   Updated: 2023/11/26 20:16:09 by fra           ########   odam.nl         */
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

	if (this->_header != nullptr)
		this->_freeNodes();
	memset(buffer, 0, HEADER_MAX_SIZE + 1);
	readChar = read(connfd, buffer, HEADER_MAX_SIZE);
	if (readChar <= 0)
		throw(ServerException("error: socket not available or empty"));
	eoh = strstr(buffer, "\\r\\n\\r\\n");
	if ((eoh == NULL) or (eoh == buffer))
		throw(ServerException("error: invalid header"));
	eoh[4] = '\0';
	this->parseHeader(buffer);
	this->_body = eoh + 8;
	if (readChar == HEADER_MAX_SIZE)
		this->parseBody(connfd);
	// std::cout << "header req: " << this->_httpReq << "\n";
	// for (auto tmp = this->_header; tmp != nullptr; tmp = tmp->next)
	// 	std::cout << "\tkey: " << tmp->key << " value: " << tmp->content << "\n";
	// std::cout << "body: " << this->_body << "\n";
}

void	HTTPparser::parseHeader( char *buffer )
{
	char *eol, *line;

	eol = strstr(buffer, "\\r\\n");
	eol[0] = '\0';
	this->_httpReq = buffer;
	line = eol + 3;
	while (true)
	{
		eol = strstr(line, "\\r\\n");
		if (eol == NULL)
			break;
		eol[0] = '\0';
		_addNode(line);
		line = eol + 3;
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
			throw(ServerException("error: socket not available"));
		buffer[readChar] = '\0';
		this->_body += buffer;
		if (readChar < 1)
			break ;
	}
}

void	HTTPparser::_addNode( char* nodeContent )
{
	char		*colon;
	HTTPlist_t	*newNode, *tmp;

	colon = strchr(nodeContent, ':');
	if (colon == nullptr)
		throw(ServerException("error: bad format header line"));
	newNode = new HTTPlist_t();
	*colon = '\0';
	newNode->key = nodeContent;
	newNode->content = colon + 2;
	newNode->next = nullptr;
	if (this->_header == nullptr)
		this->_header = newNode;
	else
	{
		tmp = this->_header;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = newNode;
	}
}

void	HTTPparser::_freeNodes( void )
{
	HTTPlist_t	*toDrop;
	while (this->_header)
	{
		toDrop = this->_header;
		this->_header = this->_header->next;
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
