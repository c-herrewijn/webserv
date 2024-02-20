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

Parameters::Parameters(void) { this->root = DEF_ROOT; }

Parameters::~Parameters(void)
{

}

Parameters::Parameters(const Parameters& copy) :
	block_index(copy.block_index),
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
	block_index = assign.block_index;
	max_size = assign.max_size;
	autoindex = assign.autoindex;
	indexes = assign.indexes;
	root = assign.root;
	error_pages = assign.error_pages;
	returns = assign.returns;
	return (*this);
}

void	Parameters::_parseRoot(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front() == ";")
		throw ParserException({"'root' can't have an empty parameter"});
	setRoot(block.front());
	block.erase(block.begin());
	if (block.front() != ";")
		throw ParserException({"'root' can't have multiple parameters '" + block.front() + "'"});
	block.erase(block.begin());
}

void	Parameters::_parseBodySize(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front() == ";")
		throw ParserException({"'client_max_body_size' can't have an empty parameter"});
	if (std::isdigit(block.front().front()) == 0)
		throw ParserException({"'client_max_body_size' must have a digit as first value in parameter"});
	errno = 0;
	char*	endPtr = NULL;
	long convertedValue = std::strtol(block.front().c_str(), &endPtr, 10);
	if ((errno == ERANGE && (convertedValue == LONG_MAX || convertedValue < 0)) ||
		(errno != 0 && convertedValue == 0))
		throw ParserException({"'" + block.front() + "' resulted in overflow or underflow\n'client_max_body_size' must be formated as '(unsigned int)/(type=K|M|G)'"});
	else if (endPtr == block.front())
		throw ParserException({"'client_max_body_size' must be formated as '(unsigned int)|(type=K||M||G)'"});
	if (endPtr and *endPtr and *endPtr != 'K' and *endPtr != 'M' and *endPtr != 'G')
	{
		std::cout << "|" << (int) *endPtr << "|\n";
		throw ParserException({"'client_max_body_size' must be formated as '(unsigned int)|(type=K||M||G)'"});
	}
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
		throw ParserException({"'index' can't have an empty parameter"});
	while (1)
	{
		if (block.front().find_first_of(" ") != std::string::npos)
			throw ParserException({"'index' parameter '" + block.front() + "' has a space in it"});
		addIndex(block.front());
		block.erase(block.begin());
		if (block.front() == ";")
		{
			block.erase(block.begin());
			break ;
		}
	}
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
	if (block.front() != ";")
	    returns[(size_t)code] = block.front();
	else
		returns[(size_t)code] = "";
	block.erase(block.begin());
	if (block.front() != ";")
		throw ParserException({"'return' keyword can have maximum 2 parameters"});
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

size_t Parameters::getMaxSize(void) const
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

void	Parameters::setSize(long val, char *order)
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
	os << std::string(indentation, '\t') << "index";
	const auto& indexes = params.getIndexes();
	for (const auto& index : indexes)
		os << " " << index;
	os << ";\n";
	const auto& errorPages = params.getErrorPages();
	for (const auto& entry : errorPages)
	{
		os << std::string(indentation, '\t') << "error_page " << entry.first << " " << entry.second << ";\n";
	}

	const auto& returns = params.getReturns();
	for (const auto& entry : returns) {
		os << std::string(indentation, '\t') << "return " << entry.first << " " << entry.second << ";\n";
	}

	return os;
}