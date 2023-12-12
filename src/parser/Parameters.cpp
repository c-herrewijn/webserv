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
		throw ErrorCatch("'root' can't have multiple parameters '" + block.front() + "'");
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
	if (block.front() == ";")
		throw ErrorCatch("'index' can't have an empty parameter");
	while (1)
	{
		if (block.front().find_first_of(" ") != std::string::npos)
			throw ErrorCatch("'index' parameter \"" + block.front() + "\" has a space in it");
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
	block.erase(block.begin());
	if (block.front() == ";")
		throw ErrorCatch("'error_page' can't have an empty parameter");
	int code;
	try {
		code = std::stoi(block.front());
		if (code < 100 || code > 599)
			throw std::out_of_range("Value is not in the range of 100-599");
		block.erase(block.begin());
	} catch (const std::invalid_argument& e) {
		throw ErrorCatch("Error code is not a valid integer");
	} catch (const std::out_of_range& e) {
		throw ErrorCatch("Given value is out of range: " + block.front());
	}
	if (block.front() == ";")
		throw ErrorCatch("After error code expected a file");
	if (block.front().front() != '/')
		throw ErrorCatch("Error code file must start with a '/':" + block.front());
	error_pages[code] = block.front();
	block.erase(block.begin());
	if (block.front() != ";")
		throw ErrorCatch("Error page can only contain 2 arguments");
	block.erase(block.begin());
}

void	Parameters::parseReturn(std::vector<std::string>& block)
{
	int code;
	block.erase(block.begin());
	// return /old-url /new-url; is not valid right now
	try {
		code = std::stoi(block.front());
		if (code < 100 || code > 599)
			throw std::out_of_range("Value is not in the range of 100-599");
		block.erase(block.begin());
	} catch (const std::invalid_argument& e) {
		throw ErrorCatch("Input is not a valid integer: '" + block.front() + "'");
	} catch (const std::out_of_range& e) {
		throw ErrorCatch("Given value is out of range: " + block.front());
	}
	if (block.front() != ";")
	    returns[(size_t)code] = block.front();
	else
		returns[(size_t)code] = "";
	block.erase(block.begin());
	if (block.front() != ";")
		throw ErrorCatch("'return' keyword can have maximum 2 parameters");
	block.erase(block.begin());
}

void	Parameters::addIndex(const std::string& val)
{
	if (indexes.find(val) == indexes.end())
		indexes.insert(val);
}

const std::unordered_set<std::string>& Parameters::getIndexes(void) const
{
	return (indexes);
}

const std::pair<size_t, char>& Parameters::getMaxSize(void) const
{
	return (max_size);
}

const std::unordered_map<size_t, std::string>& Parameters::getErrorPages(void) const
{
	return (error_pages);
}

const std::unordered_map<size_t, std::string>& Parameters::getReturns(void) const
{
	return (returns);
}

const bool& Parameters::getAutoindex(void) const
{
	return (autoindex);
}

const std::string& Parameters::getRoot(void) const
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

std::ostream& operator<<(std::ostream& os, const Parameters& params)
{
	os << "Root: " << params.getRoot() << "\n";
	os << "Max Size: " << params.getMaxSize().first << ", " << params.getMaxSize().second << "\n";
	os << "Autoindex: " << (params.getAutoindex() ? "true" : "false") << "\n";

	os << "Indexes: ";
	const auto& indexes = params.getIndexes();
	for (const auto& index : indexes) {
		os << index << " ";
	}
	os << "\n";

	os << "Error Pages:" << "\n";
	const auto& errorPages = params.getErrorPages();
	for (const auto& entry : errorPages) {
		os << "  " << entry.first << ": " << entry.second << "\n";
	}

	os << "Returns:" << "\n";
	const auto& returns = params.getReturns();
	for (const auto& entry : returns) {
		os << "  " << entry.first << ": " << entry.second << "\n";
	}

	return os;
}