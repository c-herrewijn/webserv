/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:16:43 by itopchu       #+#    #+#                 */
/*   Updated: 2023/11/26 14:16:43 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(void)
{
	serverMap["listen"] = &Server::parseListen;
	serverMap["server_name"] = &Server::parseServerName;
	serverMap["location"] = &Server::parseLocation;

	paramMap["root"] = &Server::parseRoot;
	paramMap["client_max_body_size"] = &Server::parseBodySize;
	paramMap["autoindex"] = &Server::parseAutoindex;
	paramMap["index"] = &Server::parseIndex;
	paramMap["error_page"] = &Server::parseErrorPage;
	paramMap["return"] = &Server::parseReturn;

	locatMap["allowedMethods"] = &Server::parseAllowedMethod;
	locatMap["alias"] = &Server::parseAlias;
	locatMap["location"] = &Server::parseLocation;
}

Server::~Server(void) { }

Server::Server(const Server& copy) :
	listens(copy.listens),
	names(copy.names),
	location(copy.location)
{

}

Server&	Server::operator=(const Server& assign)
{
	listens.clear();
	names.clear();
	location.clear();
	listens = assign.listens;
	names = assign.names;
	location = assign.location;
	return (*this);
}

void	Server::parseListen(std::vector<std::string>& block)
{

}

void	Server::parseServerName(std::vector<std::string>& block)
{

}

void	Server::parseLocation(std::vector<std::string>& block)
{

}

void	Server::parseRoot(std::vector<std::string>& block)
{

}

void	Server::parseBodySize(std::vector<std::string>& block)
{

}

void	Server::parseAutoindex(std::vector<std::string>& block)
{

}

void	Server::parseIndex(std::vector<std::string>& block)
{

}

void	Server::parseErrorPage(std::vector<std::string>& block)
{

}

void	Server::parseReturn(std::vector<std::string>& block)
{

}

void	Server::parseAllowedMethod(std::vector<std::string>& block)
{

}

void	Server::parseAlias(std::vector<std::string>& block)
{

}

void	Server::fillServer(std::vector<std::string>& block)
{
	std::map<std::string, ParserFunction>::iterator index;

	for (std::vector<std::string>::iterator it = block.begin(); it != block.end(); it++)
	{
		index = serverMap.find(*it);
		if (index == serverMap.end())
		{
			index = locatMap.find(*it);
			if (index == locatMap.end())
				throw ErrorCatch("\"" + *it + "\" is not a valid keyword in this context");
		}
		(this->*(index->second))(block);
	}
}

bool	Server::clearEmpty(std::vector<std::string>& block)
{
	for (std::vector<std::string>::iterator it = block.begin(); it != block.end();)
	{
		if (*it == " ")
			it = block.erase(it);
		else
			++it;
	}
	return block.empty();
}

void	Server::parseBlock(std::vector<std::string>& block)
{
	if (clearEmpty(block))
		return ;
	std::cout << "-------Printing the block-------\n";
	if (block.front() != "server")
		throw ErrorCatch("First arg is not 'server'");
    block.erase(block.begin());
	if (block.front() != "{")
		throw ErrorCatch("After 'server' '{' expected");
    block.erase(block.begin());
	if (block[block.size() - 1] != "}")
		throw ErrorCatch("Last element is not '}");
	block.pop_back();
	fillServer(block);
}
