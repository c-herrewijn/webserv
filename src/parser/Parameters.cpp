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
	this->cgi_allowed = DEF_CGI_ALLOWED;
	this->cgi_extension = DEF_CGI_EXTENTION;
	for (int tmp = 0; tmp < M_SIZE; tmp++)
		allowedMethods[tmp] = 0;
	max_size = static_cast<std::uintmax_t>(DEF_SIZE) * 1024 * 1024 * 1024;

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
	allowedMethods(copy.allowedMethods),
	cgi_extension(copy.cgi_extension),
	cgi_allowed(copy.cgi_allowed)
{

}

Parameters&	Parameters::operator=(const Parameters& assign)
{
	if (this != &assign)
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
		cgi_extension = assign.cgi_extension;
		cgi_allowed = assign.cgi_allowed;
	}
	return (*this);
}

void	Parameters::_parseCgiExtension(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front().find_first_not_of("abcdefghijklmnoprstuvyzwqxABCDEFGHIJKLMNOPRSTUVYZWQX") != std::string::npos)
		throw ParserException({"Only alpha characters expected in cgi_extension: '" + block.front() + "'"});
	cgi_extension = "." + block.front();
	block.erase(block.begin());
	if (block.front() != ";")
		throw ParserException({"Unexpected element in cgi_extension: '" + block.front() + "', a ';' is expected"});
	block.erase(block.begin());
}

void	Parameters::_parseCgiAllowed(std::vector<std::string>& block)
{
	block.erase(block.begin());
	if (block.front() == "true")
		cgi_allowed = true;
	else if (block.front() == "false")
		cgi_allowed = false;
	else
		throw ParserException({"Unexpected element in cgi_allowed: '" + block.front() + "'"});
	block.erase(block.begin());
	if (block.front() != ";")
		throw ParserException({"Unexpected element in cgi_allowed: '" + block.front() + "', a ';' is expected"});
	block.erase(block.begin());
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
	setRoot(t_path(block.front()));
	block.erase(block.begin());
	if (block.front() != ";")
		throw ParserException({"'root' can't have multiple parameters '" + block.front() + "'"});
	block.erase(block.begin());
}

static void	capSize(uintmax_t& value, char* type)
{

	if (type == nullptr)
		return ;
    switch (*type) {
        case 'G':
            if (value > MAX_SIZE)
			{
                std::cerr << "Warning: Size '" + std::to_string(value) + *type + "' is capped to 20G" << std::endl;
                value = MAX_SIZE;
            }
            break;
        case 'M':
            if (value > MAX_SIZE * 1024)
			{
                std::cerr << "Warning: Size '" + std::to_string(value) + *type + "' is capped to 20G" << std::endl;
                value = MAX_SIZE * 1024;
            }
            break;
        case 'K':
            if (value > MAX_SIZE * 1024 * 1024)
			{
                std::cerr << "Warning: Size '" + std::to_string(value) + *type + "' is capped to 20G" << std::endl;
                value = MAX_SIZE * 1024 * 1024;
            }
            break;
        default:
            std::cerr << "Error: Invalid size type." << std::endl;
            break;
    }
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
	if (errno == ERANGE)
		throw ParserException({"'" + block.front() + "' resulted in overflow or underflow\n'client_max_body_size' must be formated as '(unsigned int)(type=K|M|G)'"});
	else if (endPtr == NULL || *endPtr == '\0')
		throw ParserException({"'client_max_body_size' must be formated as '(unsigned int)(type=K||M||G)': " + block.front()});
	if (!endPtr || (*endPtr != 'K' && *endPtr != 'M' && *endPtr != 'G'))
		throw ParserException({"'client_max_body_size' must be formated as '(unsigned int)(type=K||M||G)': " + block.front()});
	capSize(convertedValue, endPtr);
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
	if (block.front() != ";")
		throw ParserException({"After 'index' file a ';' expected '" + block.front() + "'"});
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

const std::string& Parameters::getCgiExtension(void) const
{
	return (cgi_extension);
}

const bool& Parameters::getCgiAllowed(void) const
{
	return (cgi_allowed);
}

const std::bitset<M_SIZE>&	Parameters::getAllowedMethods(void) const
{
	return (allowedMethods);
}

const t_path& Parameters::getIndex(void) const
{
	return (this->index);
}

std::uintmax_t Parameters::getMaxSize(void) const
{
	return (max_size);
}

const	t_string_map& Parameters::getErrorPages(void) const
{
	return (error_pages);
}

const	t_string_map& Parameters::getReturns(void) const
{
	return (returns);
}

const bool& Parameters::getAutoindex(void) const
{
	return (autoindex);
}

const t_path& Parameters::getRoot(void) const
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

void	Parameters::setRoot(t_path val)
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
	else if (block.front() == "cgi_extension")
		_parseCgiExtension(block);
	else if (block.front() == "cgi_allowed")
		_parseCgiAllowed(block);
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
