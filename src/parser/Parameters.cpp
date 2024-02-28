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

Parameters::Parameters(void)
{
	this->root = DEF_ROOT;
	for (int tmp = 0; tmp < M_SIZE; tmp++)
		allowedMethods[tmp] = 0;
}

Parameters::~Parameters(void)
{

}

Parameters::Parameters(const Parameters& copy) :
	block_index(copy.block_index),
	max_size(copy.max_size),
	autoindex(copy.autoindex),
	index(copy.index),
	root(copy.root),
	error_pages(copy.error_pages),
	returns(copy.returns),
	allowedMethods(copy.allowedMethods)
{

}

Parameters&	Parameters::operator=(const Parameters& assign)
{
	error_pages.clear();
	returns.clear();
	allowedMethods = assign.allowedMethods;
	block_index = assign.block_index;
	max_size = assign.max_size;
	autoindex = assign.autoindex;
	index = assign.index;
	root = assign.root;
	error_pages = assign.error_pages;
	returns = assign.returns;
	return (*this);
}

void	Parameters::_parseAllowedMethod(std::vector<std::string>& block)
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

void	Parameters::_parseRoot(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front() == ";")
		throw ParserException({"'root' can't have an empty parameter"});
	if (block.front().front() != '/')
		throw ParserException({"'root' must begin with a '/' '" + block.front() + "'"});
	setRoot(block.front());
	block.erase(block.begin());
	if (block.front() != ";")
		throw ParserException({"'root' can't have multiple parameters '" + block.front() + "'"});
	block.erase(block.begin());
}

// size must be stored as uint. max size can be 20G, value must be clamped 0-20G
void	Parameters::_parseBodySize(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front() == ";")
		throw ParserException({"'client_max_body_size' can't have an empty parameter"});
	if (std::isdigit(block.front().front()) == 0)
		throw ParserException({"'client_max_body_size' must have a digit as first value in parameter"});
	errno = 0;
	char*	endPtr = NULL;
	uintmax_t convertedValue = std::strtoul(block.front().c_str(), &endPtr, 10);
	if ((errno == ERANGE && (convertedValue == LONG_MAX || convertedValue < 0)) ||
		(errno != 0 && convertedValue == 0))
		throw ParserException({"'" + block.front() + "' resulted in overflow or underflow\n'client_max_body_size' must be formated as '(unsigned int)/(type=K|M|G)'"});
	else if (endPtr == block.front())
		throw ParserException({"'client_max_body_size' must be formated as '(unsigned int)|(type=K||M||G)'"});
	if (endPtr && *endPtr && *endPtr != 'K' && *endPtr != 'M' && *endPtr != 'G')
		throw ParserException({"'client_max_body_size' must be formated as '(unsigned int)|(type=K||M||G)'"});
	setSize(convertedValue, endPtr);
	block.erase(block.begin());
	if (block.front() != ";")
		throw ParserException({"'client_max_body_size' can't have multiple parameters"});
	block.erase(block.begin());
}

void	Parameters::_parseAutoindex(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front() == ";")
		throw ParserException({"'autoindex' can't have an empty parameter"});
	if (block.front() == "on")
		setAutoindex(true);
	else if (block.front() == "off")
		setAutoindex(false);
	else
		throw ParserException({"'autoindex' can only have 'on' or 'off' as parameter"});
	block.erase(block.begin());
	if (block.front() != ";")
		throw ParserException({"'autoindex' can't have multiple parameters"});
	block.erase(block.begin());
}

void	Parameters::_parseIndex(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front() == ";")
		throw ParserException({"After index expected a file"});
	if (block.front().front() != '/')
		throw ParserException({"File name for index must start with a '/': " + block.front()});
	if (block.front().find_first_of('/') != block.front().find_last_of('/'))
		throw ParserException({"'index' must be file '" + block.front() + "'"});
	this->index = block.front();
	block.erase(block.begin());
}

void	Parameters::_parseErrorPage(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front() == ";")
		throw ParserException({"'error_page' can't have an empty parameter"});
	int code;
	try {
		code = std::stoi(block.front());
		if (code < 100 || code > 599)
			throw std::out_of_range("value is not in the range of 100-599");
		block.erase(block.begin());
	} catch (const std::invalid_argument& e) {
		throw ParserException({"error_page code is not a valid integer '" + block.front() + "'"});
	} catch (const std::out_of_range& e) {
		throw ParserException({"error_page code is out of range: '" + block.front() + "'"});
	}
	if (block.front() == ";")
		throw ParserException({"After error_page code expected a file '" + block.front() + "'"});
	if (block.front().front() != '/')
		throw ParserException({"File name for error_page must start with a '/': " + block.front()});
	if (block.front().find_first_of('/') != block.front().find_last_of('/'))
		throw ParserException({"'error_page' must be file '" + block.front() + "'"});
	error_pages[code] = block.front();
	block.erase(block.begin());
	if (block.front() != ";")
		throw ParserException({"error_page can only contain 2 arguments: '" + block.front() + "'"});
	block.erase(block.begin());
}

void	Parameters::_parseReturn(std::vector<std::string>& block)
{
	int code;
	block.erase(block.begin());
	// return /old-url /new-url; is not valid right now
	try {
		code = std::stoi(block.front());
		if (code < 100 || code > 599)
			throw std::out_of_range("value is not in the range of 100-599");
		block.erase(block.begin());
	} catch (const std::invalid_argument& e) {
		throw ParserException({"input is not a valid integer: '" + block.front() + "'"});
	} catch (const std::out_of_range& e) {
		throw ParserException({"given value is out of range: " + block.front()});
	}
	if (block.front() == ";")
		throw ParserException({"After return code expected a file '" + block.front() + "'"});
	if (block.front().front() != '/')
		throw ParserException({"File name for return must start with a '/': " + block.front()});
	if (block.front().find_first_of('/') != block.front().find_last_of('/'))
		throw ParserException({"'return' must be file '" + block.front() + "'"});
	returns[(size_t)code] = block.front();
	block.erase(block.begin());
	if (block.front() != ";")
		throw ParserException({"'return' keyword must have 2 parameters"});
	block.erase(block.begin());
}

const std::bitset<M_SIZE>&	Parameters::getAllowedMethods(void) const
{
	return (allowedMethods);
}

const std::string& Parameters::getIndex(void) const
{
	return (this->index);
}

std::uintmax_t Parameters::getMaxSize(void) const
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

// this must be updated. size must be stored as bytes
void	Parameters::setSize(uintmax_t val, char *order)
{
	this->max_size = val;

	if (order == nullptr)
		return ;
	switch (*order)
	{
		case 'G':
			this->max_size *= 1024;
			[[fallthrough]];
		case 'M':
			this->max_size *= 1024;
			[[fallthrough]];
		case 'K':
			this->max_size *= 1024;
	}
}

void	Parameters::setRoot(std::string& val)
{
	root = val;
}

void	Parameters::fill(std::vector<std::string>& block)
{
	if (block.front() == "root")
		_parseRoot(block);
	else if (block.front() == "client_max_body_size")
		_parseBodySize(block);
	else if (block.front() == "autoindex")
		_parseAutoindex(block);
	else if (block.front() == "index")
		_parseIndex(block);
	else if (block.front() == "error_page")
		_parseErrorPage(block);
	else if (block.front() == "return")
		_parseReturn(block);
	else if (block.front() == "allowMethods")
		_parseAllowedMethod(block);
	else
		throw ParserException({"'" + block.front() + "' is not a valid parameter"});
}

void Parameters::setBlockIndex(size_t ref)
{
	this->block_index = ref + 1;
}

const size_t& Parameters::getBlockIndex(void) const
{
	return (this->block_index);
}

std::ostream& operator<<(std::ostream& os, const Parameters& params)
{
    size_t indentation = params.getBlockIndex();
	os << std::string(indentation, '\t') << "root " << params.getRoot() << ";\n";
	os << std::string(indentation, '\t') << "client_max_body_size " << params.getMaxSize() << ";\n";
	os << std::string(indentation, '\t') << "autoindex " << (params.getAutoindex() ? "true" : "false") << ";\n";
	os << std::string(indentation, '\t') << "index " << params.getIndex() << ";\n";
    // Print allowMethods
    os << std::string(indentation + 1, '\t') << "allowMethods " << params.allowedMethods << ";\n";

	const auto& errorPages = params.getErrorPages();
	for (const auto& entry : errorPages)
		os << std::string(indentation, '\t') << "error_page " << entry.first << " " << entry.second << ";\n";

	const auto& returns = params.getReturns();
	for (const auto& entry : returns)
		os << std::string(indentation, '\t') << "return " << entry.first << " " << entry.second << ";\n";
	return os;
}