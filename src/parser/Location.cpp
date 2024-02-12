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
	block_index = 0;
	for (int tmp = 0; tmp < M_SIZE; tmp++)
		allowedMethods[tmp] = 0;
}

Location::~Location(void)
{
	nested.clear();
}

Location::Location(const Location& copy) :
	block_index(copy.block_index),
	allowedMethods(copy.allowedMethods),
	URL(copy.URL),
	alias(copy.alias),
	params(copy.params),
	nested(copy.nested)
{

}

Location&	Location::operator=(const Location& assign)
{
	block_index = assign.block_index;
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
	block_index = param.getBlockIndex();
	params = param;
	params.setBlockIndex(param.getBlockIndex());
	block.erase(block.begin());
	if (block.front()[0] != '/')
		throw ParserException({"after 'location' expected a /URL"});
	URL = block.front();
	block.erase(block.begin());
	if (block.front() != "{")
		throw ParserException({"after '/URL' expected a '{'"});
	block.erase(block.begin());
	while (block.front() != "}")
	{
		if (block.front() == "alias")
			_parseAlias(block);
		else if (block.front() == "allowMethods")
			_parseAllowedMethod(block);
		else if (block.front() == "location")
		{
			Location local(block, params);
			local.setBlockIndex(this->block_index);
			nested.push_back(local);
		}
		else if (block.front() == "root" || block.front() == "client_max_body_size" ||
				block.front() == "autoindex" || block.front() == "index" ||
				block.front() == "error_page" || block.front() == "return")
			params.fill(block);
		else
			throw ParserException({"'" + block.front() + "' is not a valid parameter in 'location' context"});
	}
	block.erase(block.begin());
}

void	Location::_parseAllowedMethod(std::vector<std::string>& block)
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
			throw ParserException({"'" + block.front() + "' is not a valid element in allowMethods parameters"});
	}
	block.erase(block.begin());
}

void	Location::_parseAlias(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front().find_first_of(" ") != std::string::npos)
		throw ParserException({"unwanted space found in '" + block.front() + "' while parsing alias"});
	if (block.front()[0] != '/')
		throw ParserException({"improper alias without '/' found on '" + block.front() + "'"});
	alias = block.front();
	block.erase(block.begin());
	if (block.front() != ";")
		throw ParserException({"after first element expected a ';' with alias elements. Error on '" + block.front() + "'"});
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

const std::string& Location::getAlias(void) const
{
	return (alias);
}

const std::string& Location::getURL(void) const
{
	return (URL);
}

void Location::setBlockIndex(const size_t& ref)
{
	this->block_index = ref + 1;
}

const size_t& Location::getBlockIndex(void) const
{
	return (this->block_index);
}

std::ostream& operator<<(std::ostream& os, const Location& location)
{
    size_t indentation = location.getBlockIndex();
    // Print the opening line for the current location
    os << std::string(indentation, '\t') << "location " << location.getURL() << " {\n";

    // Print alias and allowMethods
    os << std::string(indentation + 1, '\t') << "alias " << location.getAlias() << ";\n";
    os << std::string(indentation + 1, '\t') << "allowMethods " << location.getAllowedMethods() << ";\n";

    // Print location params
    os << location.getParams();

    // Print Nested Locations
    const auto& nestedLocations = location.getNested();
	for (const auto& nested : nestedLocations) {
		// Recursively call operator<< for each nested location
		os << nested;
	}

    // Print the closing line for the current location
    os << std::string(indentation, '\t') << "}\n";
    
    return os;
}
