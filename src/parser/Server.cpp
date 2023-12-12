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

}

Server::~Server(void)
{
	listens.clear();
	names.clear();
	names.clear();
	locations.clear();
}

Server::Server(const Server& copy) :
	listens(copy.listens),
	names(copy.names),
	params(copy.params),
	locations(copy.locations)
{

}

Server&	Server::operator=(const Server& assign)
{
	listens.clear();
	names.clear();
	locations.clear();
	listens = assign.listens;
	names = assign.names;
	locations = assign.locations;
	params = assign.params;
	return (*this);
}

void	Server::parseListen(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front() == ";")
		throw ErrorCatch("Can't use ';' after keyword 'listen'");
	if (block.front() == "default_server")
		throw ErrorCatch("Before 'default_server' an ip/port expected");
	Listen tmp;
	tmp.fillValues(block);
	if (block.front() == "default_server")
	{
		tmp.setDef(true);
		block.erase(block.begin());
	}
	listens.push_back(tmp);
	if (block.front() != ";")
		throw ErrorCatch("Missing semicolumn on Listen, before: '" + block.front() + "'");
	block.erase(block.begin());
}

void	Server::parseServerName(std::vector<std::string>& block)
{
	// THIS PART IS SUS. What about asterix?
	block.erase(block.begin());
	for (std::vector<std::string>::iterator it = block.begin(); it != block.end();)
	{
		if (*it == ";")
		{
			block.erase(block.begin());
			break ;
		}
		if (block.front().find_first_not_of("abcdefghijklmnoprstuvyzwxqABCDEFGHIJKLMNOPRSTUVYZWXQ0123456789-.") != std::string::npos)
			throw ErrorCatch("Only 'alpha' 'digit' '-' and '.' characters are accepted in 'server_name'");
		names.push_back(block.front());
		block.erase(block.begin());
	}
}

void	Server::parseLocation(std::vector<std::string>& block)
{
	Location	local(block, params);
	locations.push_back(local);
}

void	Server::fillServer(std::vector<std::string>& block)
{
	// Parser keyword separations
	for (std::vector<std::string>::iterator it = block.begin(); it != block.end();)
	{
		if (*it == "listen")
			parseListen(block);
		else if (*it == "server_name")
			parseServerName(block);
		else if (*it == "location")
			parseLocation(block);
		else if (*it == "allowMethods" || *it == "root" ||
				*it == "client_max_body_size" || *it == "autoindex" ||
				*it == "index" || *it == "error_page" || *it == "return")
			params.fill(block);
		else
			throw ErrorCatch("\"" + block.front() + "\" is not a valid parameter");
	}
}

void	Server::parseBlock(std::vector<std::string>& block)
{
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

const std::vector<Listen>& Server::getListens(void) const
{
	return (listens);
}

const std::vector<std::string>& Server::getNames(void) const
{
	return (names);
}

const Parameters&	Server::getParams(void) const
{
	return (params);
}

const std::vector<Location>&	Server::getLocations() const
{
	return (locations);
}

std::ostream& operator<<(std::ostream& os, const Server& server)
{
    const auto& listens = server.getListens();
    os << "\nListens:" "\n";
    for (const auto& listen : listens) {
        os << listen << "\n";
    }

    os << "\nServer Names:" "\n";
    const auto& names = server.getNames();
    for (const auto& name : names) {
        os << name << "\n";
    }

    os << "\nParameters:" << "\n";
    os << server.getParams() << "\n";

    os << "\nLocations:" "\n";
    const auto& locations = server.getLocations();
    for (const auto& location : locations) {
        os << location;
    }

    return os;
}