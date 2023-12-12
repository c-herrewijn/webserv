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
	locations(copy.locations),
	cgi_directory(copy.cgi_directory),
	cgi_extension(copy.cgi_extension),
	cgi_allowed(copy.cgi_allowed)
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
	cgi_directory = assign.cgi_directory;
	cgi_extension = assign.cgi_extension;
	cgi_allowed = assign.cgi_allowed;
	return (*this);
}

void	Server::parseCgiDir(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front().find(' ') != std::string::npos)
		throw ErrorCatch("Unexpected element in cgi_directory: '" + block.front() + "'");
	if (block.front().front() != '/')
		throw ErrorCatch("Directories must begin with a '/''" + block.front() + "'");
	cgi_directory = block.front();
	block.erase(block.begin());
	if (block.front() != ";")
		throw ErrorCatch("Unexpected element in cgi_directory: '" + block.front() + "', a ';' is expected");
	block.erase(block.begin());
}

void	Server::parseCgiExtension(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front().find(' ') != std::string::npos)
		throw ErrorCatch("Unexpected element in cgi_extension: '" + block.front() + "'");
	cgi_extension = block.front();
	block.erase(block.begin());
	if (block.front() != ";")
		throw ErrorCatch("Unexpected element in cgi_extension: '" + block.front() + "', a ';' is expected");
	block.erase(block.begin());
}

void	Server::parseCgiAllowed(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front() == "true")
		cgi_allowed = true;
	else if (block.front() == "false")
		cgi_allowed = false;
	else
		throw ErrorCatch("Unexpected element in cgi_allowed: '" + block.front() + "'");
	block.erase(block.begin());
	if (block.front() != ";")
		throw ErrorCatch("Unexpected element in cgi_allowed: '" + block.front() + "', a ';' is expected");
	block.erase(block.begin());
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
		else if (block.front() == "cgi_directory")
			parseCgiDir(block);
		else if (block.front() == "cgi_extension")
			parseCgiExtension(block);
		else if (block.front() == "cgi_allowed")
			parseCgiAllowed(block);
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

const std::string& Server::getCgiDir(void) const
{
	return (cgi_directory);
}

const std::string& Server::getCgiExtension(void) const
{
	return (cgi_extension);
}

const bool& Server::getCgiAllowed(void) const
{
	return (cgi_allowed);
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
	os << "\nCgi params:\n";
	os << "\tCgi allowed: " << server.getCgiAllowed();
	os << "\n\tCgi directory: " << server.getCgiDir();
	os << "\n\tCgi extension: " << server.getCgiExtension() << "\n";
	os << "----------------\n";
    os << "\nListens:\n";
    for (const auto& listen : listens) {
        os << listen << "\n";
    }
	os << "----------------\n";
    os << "\nServer Names:" "\n";
    const auto& names = server.getNames();
    for (const auto& name : names) {
        os << "\t" << name << "\n";
    }
	os << "----------------\n";
    os << "\nParameters\n" << "\n";
    os << server.getParams() << "\n";
	os << "----------------\n";
    os << "\nLocations:" "\n";
    const auto& locations = server.getLocations();
    for (const auto& location : locations) {
        os << location;
    }
	os << "--------END--------\n";

    return os;
}