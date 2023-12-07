/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Location.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/03 00:06:14 by itopchu       #+#    #+#                 */
/*   Updated: 2023/12/03 00:06:14 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location(void)
{
	for (int tmp = 0; tmp < M_SIZE; tmp++)
		allowedMethods[tmp] = 0;
}

Location::~Location(void)
{
	nested.clear();
}

Location::Location(const Location& copy) :
	allowedMethods(copy.allowedMethods),
	URL(copy.URL),
	alias(copy.alias),
	params(copy.params),
	nested(copy.nested)
{

}

Location&	Location::operator=(const Location& assign)
{
	allowedMethods = assign.allowedMethods;
	URL = assign.URL;
	alias = assign.alias;
	params = assign.params;
	nested.clear();
	nested = assign.nested;
	return (*this);
}

Location::Location(std::vector<std::string>& block, const Parameters& param)
{
	params = param;
	block.erase(block.begin());
	if (block.front()[0] != '/')
		throw ErrorCatch("After 'location' expected a /URL");
	URL = block.front();
	block.erase(block.begin());
	if (block.front() != "{")
		throw ErrorCatch("After '/URL' expected a '{'");
	block.erase(block.begin());
	while (block.front() != "}")
	{
		if (block.front() == "alias")
			parseAlias(block);
		else if (block.front() == "allowMethods")
			parseAllowedMethod(block);
		else if (block.front() == "location")
		{
			Location local(block, params);
			nested.push_back(local);
		}
		else if (block.front() == "root" || block.front() == "client_max_body_size" ||
				block.front() == "autoindex" || block.front() == "index" ||
				block.front() == "error_page" || block.front() == "return")
			params.fill(block);
		else
			throw ErrorCatch("\'" + block.front() + "\' is not a valid parameter in 'location' context");
	}
	block.erase(block.begin());
}

void	Location::parseAllowedMethod(std::vector<std::string>& block)
{
	block.erase(block.begin());
	while (1)
	{
		if (block.front() == "GET")
		{
			allowedMethods[M_GET] = 1;
			block.erase(block.begin());
		}
		else if (block.front() == "POST")
		{
			allowedMethods[M_POST] = 1;
			block.erase(block.begin());
		}
		else if (block.front() == "DELETE")
		{
			allowedMethods[M_DELETE] = 1;
			block.erase(block.begin());
		}
		else if (block.front() == ";")
			break ;
		else
			throw ErrorCatch("'" + block.front() + "' is not a valid element in allowMethods parameters");
	}
	block.erase(block.begin());
}

void	Location::parseAlias(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front().find_first_of(" ") != std::string::npos)
		throw ErrorCatch("Unwanted space found in '" + block.front() + "' while parsing alias");
	if (block.front()[0] != '/')
		throw ErrorCatch("Improper alias without '/' found on '" + block.front() + "'");
	alias = block.front();
	block.erase(block.begin());
	if (block.front() != ";")
		throw ErrorCatch("After first element expected a ';' with alias elements. Error on '" + block.front() + "'");
	block.erase(block.begin());
}

const std::vector<Location>& Location::getNested(void) const
{
	return (nested);
}

const Parameters&	Location::getParams(void) const
{
	return (params);
}

const std::bitset<M_SIZE>&	Location::getAllowedMethods(void) const
{
	return (allowedMethods);
}

std::ostream& operator<<(std::ostream& os, const Location& location) {
    os << "URL: " << location.URL << "\n";
    os << "Alias: " << location.alias << "\n";
    os << "Allowed Methods: " << location.allowedMethods << "\n";
    os << "Params: " << location.params << "\n";
    os << "Nested Locations:" << "\n";
    const auto& nestedLocations = location.getNested();
    for (const auto& nested : nestedLocations) {
        os << nested;
    }

    return os;
}