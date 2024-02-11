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
	if (this != &assign)
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
	}
	return (*this);
}

Server::~Server(void)
{
	listens.clear();
	names.clear();
	names.clear();
	locations.clear();
}

void	Server::parseBlock(std::vector<std::string>& block)
{
	if (block.front() != "server")
		throw ErrorCatch("First arg is not 'server'");
    block.erase(block.begin());
	if (block.front() != "{")
		throw ErrorCatch("After 'server' a '{' expected");
    block.erase(block.begin());
	if (block[block.size() - 1] != "}")
		throw ErrorCatch("Last element is not a '}");
	block.pop_back();
	_fillServer(block);
}

const std::vector<Listen>& Server::getListens(void) const
{
	return (listens);
}

const std::vector<std::string>& Server::getNames(void) const
{
	return (names);
}

const std::string&		Server::getPrimaryName(void) const
{
	return (names[0]);
}

const Parameters&	Server::getParams(void) const
{
	return (params);
}

const std::vector<Location>&	Server::getLocations() const
{
	return (locations);
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

void	Server::_parseListen(std::vector<std::string>& block)
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

void	Server::_parseServerName(std::vector<std::string>& block)
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

void	Server::_parseLocation(std::vector<std::string>& block)
{
	Location	local(block, params);
	locations.push_back(local);
}

void	Server::_parseCgiDir(std::vector<std::string>& block)
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

void	Server::_parseCgiExtension(std::vector<std::string>& block)
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

void	Server::_parseCgiAllowed(std::vector<std::string>& block)
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

void	Server::_fillServer(std::vector<std::string>& block)
{
	params.setBlockIndex(0);
	// Parser keyword separations
	for (std::vector<std::string>::iterator it = block.begin(); it != block.end();)
	{
		if (*it == "listen")
			_parseListen(block);
		else if (*it == "server_name")
			_parseServerName(block);
		else if (*it == "location")
			_parseLocation(block);
		else if (block.front() == "cgi_directory")
			_parseCgiDir(block);
		else if (block.front() == "cgi_extension")
			_parseCgiExtension(block);
		else if (block.front() == "cgi_allowed")
			_parseCgiAllowed(block);
		else
			params.fill(block);
	}
}

std::ostream& operator<<(std::ostream& os, const Server& server) {
    os << "server {\n";

    // Print Listens
    const auto& listens = server.getListens();
    for (const auto& listen : listens) {
        os << "\t" << listen;
    }

    os << "\tserver_name";
    const auto& names = server.getNames();
    for (const auto& name : names) {
        os << " " << name;
    }
    os << ";\n";

    os << "\tcgi_extension " << server.getCgiExtension() << ";\n";
    os << "\tcgi_directory " << server.getCgiDir() << ";\n";
    os << "\tcgi_allowed " << (server.getCgiAllowed() ? "true" : "false") << ";\n";

    os << "\troot " << server.getParams().getRoot() << ";\n";
    os << "\tclient_max_body_size " << server.getParams().getMaxSize().first << server.getParams().getMaxSize().second << ";\n";
    os << "\tautoindex " << (server.getParams().getAutoindex() ? "on" : "off") << ";\n";

    const auto& indexes = server.getParams().getIndexes();
    for (const auto& index : indexes) {
        os << "\tindex " << index << ";\n";
    }

    const auto& errorPages = server.getParams().getErrorPages();
    for (const auto& entry : errorPages) {
        os << "\terror_page " << entry.first << " " << entry.second << ";\n";
    }

    const auto& returns = server.getParams().getReturns();
    for (const auto& entry : returns) {
        os << "\treturn " << entry.first << " " << entry.second << ";\n";
    }

    const auto& locations = server.getLocations();
    for (const auto& location : locations) {
        os  << location;
    }
    os << "}\n";
    return os;
}
