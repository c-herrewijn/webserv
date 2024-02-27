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
	params(copy.params),
	nested(copy.nested)
{

}

Location&	Location::operator=(const Location& assign)
{
	block_index = assign.block_index;
	allowedMethods = assign.allowedMethods;
	URL = assign.URL;
	params = assign.params;
	nested.clear();
	nested = assign.nested;
	return (*this);
}

Location::Location(std::vector<std::string>& block, const Parameters& param)
{
	std::vector<std::vector<std::string>> locationHolder;
	std::vector<std::string>::iterator index;
	uint64_t size = 0;
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
	while (block.front() != "}" && !block.empty())
	{
		if (block.front() == "allowMethods")
			_parseAllowedMethod(block);
		else if (block.front() == "location")
		{
			index = block.begin();
			while (index != block.end() && *index != "{")
				index++;
			if (index == block.end())
				throw ParserException({"Error on location parsing"});
			index++;
			size++;
			while (size && index != block.end())
			{
				if (*index == "{")
					size++;
				else if (*index == "}")
					size--;
				index++;
			}
			if (size)
				throw ParserException({"Error on location parsing with brackets"});
			std::vector<std::string> subVector(block.begin(), index);
			block.erase(block.begin(), index);
			locationHolder.push_back(subVector);
		}
		else if (block.front() == "root" || block.front() == "client_max_body_size" ||
				block.front() == "autoindex" || block.front() == "index" ||
				block.front() == "error_page" || block.front() == "return")
			params.fill(block);
		else
			throw ParserException({"'" + block.front() + "' is not a valid parameter in 'location' context"});
	}
	block.erase(block.begin());
	for (std::vector<std::vector<std::string>>::iterator it = locationHolder.begin(); it != locationHolder.end(); it++)
	{
		Location local(*it, params);
		local.setBlockIndex(this->block_index);
		nested.push_back(local);
	}
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

    // Print allowMethods
    os << std::string(indentation + 1, '\t') << "allowMethods " << location.allowedMethods << ";\n";

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
