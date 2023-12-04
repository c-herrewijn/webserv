/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parameters.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/03 00:05:57 by itopchu       #+#    #+#                 */
/*   Updated: 2023/12/03 00:05:57 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Parameters.hpp"

Parameters::Parameters(void) { }

Parameters::~Parameters(void)
{

}

Parameters::Parameters(const Parameters& copy) :
	max_size(copy.max_size),
	autoindex(copy.autoindex),
	indexes(copy.indexes),
	root(copy.root),
	error_pages(copy.error_pages),
	returns(copy.returns)
{

}

Parameters&	Parameters::operator=(const Parameters& assign)
{
	indexes.clear();
	error_pages.clear();
	returns.clear();
	max_size = assign.max_size;
	autoindex = assign.autoindex;
	indexes = assign.indexes;
	root = assign.root;
	error_pages = assign.error_pages;
	returns = assign.returns;
	return (*this);
}

void	Parameters::parseRoot(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front() == ";")
		throw ErrorCatch("'root' can't have an empty parameter");
	setRoot(block.front());
	block.erase(block.begin());
	if (block.front() != ";")
		throw ErrorCatch("'root' can't have multiple parameters");
	block.erase(block.begin());
}

void	Parameters::parseBodySize(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front() == ";")
		throw ErrorCatch("'client_max_body_size' can't have an empty parameter");
	if (std::isdigit(block.front().front()) == 0)
		throw ErrorCatch("'client_max_body_size' must have a digit as firt value in parameter");
	errno = 0;
	char*	endPtr = NULL;
	long convertedValue = std::strtol(block.front().c_str(), &endPtr, 10);
	if ((errno == ERANGE && (convertedValue == LONG_MAX || convertedValue == LONG_MIN)) ||
		(errno != 0 && convertedValue == 0))
		throw ErrorCatch("\"" + block.front() + "\" resulted in overflow or underflow\n'client_max_body_size' must be formated as '(unsigned int)/(type=K|M|G)'");
	else if (endPtr == block.front())
		throw ErrorCatch("'client_max_body_size' must be formated as '(unsigned int)|(type=K||M||G)'");
	if (endPtr)
	{
		if (endPtr + 1 && (*endPtr != 'K' && *endPtr != 'M' && *endPtr != 'G'))
			throw ErrorCatch("'client_max_body_size' must be formated as '(unsigned int)|(type=K||M||G)'");
		setSize(convertedValue, *endPtr);
	}
	else
		setSize(convertedValue, (int)DEF_SIZE_TYPE);
	block.erase(block.begin());
	if (block.front() != ";")
		throw ErrorCatch("'client_max_body_size' can't have multiple parameters");
	block.erase(block.begin());
}

void	Parameters::parseAutoindex(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front() == ";")
		throw ErrorCatch("'autoindex' can't have an empty parameter");
	if (block.front() == "on")
		setAutoindex(true);
	else if (block.front() == "off")
		setAutoindex(false);
	else
		throw ErrorCatch("'autoindex' can only have 'on' or 'off' as parameter");
	block.erase(block.begin());
	if (block.front() != ";")
		throw ErrorCatch("'autoindex' can't have multiple parameters");
	block.erase(block.begin());
}

void	Parameters::parseIndex(std::vector<std::string>& block)
{
	block.erase(block.begin());
	while (1)
	{
		if (block.front() == ";")
			throw ErrorCatch("'index' can't have an empty parameter");
		if (block.front().find_first_of(" ") != std::string::npos)
			throw ErrorCatch("'index' parameter \"" + block.front() + "\" has a space in it");
		std::unordered_set<std::string> tmp = getIndexes();
		if (tmp.find(block.front()) == tmp.end())
			addIndex(block.front());
		block.erase(block.begin());
		if (block.front() == ";")
		{
			block.erase(block.begin());
			break ;
		}
	}
}

void	Parameters::parseErrorPage(std::vector<std::string>& block)
{
	// THIS PART IS NOT DONE YET
	block.erase(block.begin());
	while (1)
	{
		if (block.front() == ";")
		{
			block.erase(block.begin());
			break ;
		}
		block.erase(block.begin());
	}
}

void	Parameters::parseReturn(std::vector<std::string>& block)
{
	// THIS PART IS NOT DONE YET
	block.erase(block.begin());
	while (1)
	{
		if (block.front() == ";")
		{
			block.erase(block.begin());
			break ;
		}
		block.erase(block.begin());
	}
}

void	Parameters::addIndex(const std::string& val)
{
	indexes.insert(val);
}

const std::unordered_set<std::string>& Parameters::getIndexes(void)
{
	return (indexes);
}

const std::pair<size_t, char>& Parameters::getMaxSize(void)
{
	return (max_size);
}

const std::unordered_map<size_t, std::string>& Parameters::getErrorPages(void)
{
	return (error_pages);
}

const std::unordered_map<size_t, std::string>& Parameters::getReturns(void)
{
	return (returns);
}

const bool& Parameters::getAutoindex(void)
{
	return (autoindex);
}

const std::string& Parameters::getRoot(void)
{
	return (root);
}

void	Parameters::setAutoindex(bool status)
{
	autoindex = status;
}

void	Parameters::setSize(long val, int c)
{
	max_size.first = (size_t)val;
	max_size.second = (char)c;
}

void	Parameters::setRoot(std::string& val)
{
	root = val;
}

void	Parameters::fill(std::vector<std::string>& block)
{
	if (block.front() == "root")
		parseRoot(block);
	else if (block.front() == "client_max_body_size")
		parseBodySize(block);
	else if (block.front() == "autoindex")
		parseAutoindex(block);
	else if (block.front() == "index")
		parseIndex(block);
	else if (block.front() == "error_page")
		parseErrorPage(block);
	else if (block.front() == "return")
		parseReturn(block);
	else
		throw ErrorCatch("\"" + block.front() + "\" is not a valid keyword");
}
