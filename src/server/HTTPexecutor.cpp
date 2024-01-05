/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPexecutor.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:11:07 by fra           #+#    #+#                 */
/*   Updated: 2024/01/05 11:36:46 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPexecutor.hpp"

ExecException::ExecException( std::initializer_list<const char*> prompts) noexcept 
	: HTTPexception(prompts)
{
	this->_msg = "parsing error -";
	for (const char *prompt : prompts)
		this->_msg += " " + std::string(prompt);
}

const std::map<HTTPmethod, std::function<void(HTTPrequest&)> > HTTPexecutor::_methods = 
{
	{HTTP_GET, _execGET},
	{HTTP_POST, _execPOST},
	{HTTP_DELETE, _execDELETE},
};

void	HTTPexecutor::execRequest(HTTPrequest& req)
{
	_checkPath(req.head.url.path);
	try
	{
		HTTPexecutor::_methods.at(req.head.method)(req);
	}
	catch(const std::out_of_range& e) {
		throw(ExecException({"unsupported HTTP method"}));
	}
}

void	HTTPexecutor::_execGET(HTTPrequest& req)
{
	if (_isCGI(req.head.url.path) == true)
		std::cout << "CGI\n";
	else
		std::cout << "not CGI\n";
}

void	HTTPexecutor::_execPOST(HTTPrequest& req)
{
	if (_isCGI(req.head.url.path) == true)
		std::cout << "CGI\n";
	else
		std::cout << "not CGI\n";
}

void	HTTPexecutor::_execDELETE(HTTPrequest& req)
{
	(void) req;
}

void	HTTPexecutor::_checkPath(std::filesystem::path const& path)
{	
	if (path.has_filename() == false)
		throw(ExecException({"file not found"}));
	else if (access(path.c_str(), R_OK) != 0)
		throw(ExecException({"permission error"}));
	
}

bool	HTTPexecutor::_isCGI(std::filesystem::path const& path)
{
	std::filesystem::path 	baseDir;
	// char 					*cwd;

	baseDir = path.parent_path();
	if (baseDir.is_absolute())
	{
		if (std::filesystem::absolute(CGI_DIR) != baseDir)
			return (false);
	}
	else
	{

	}
	if (path.has_filename() == false)
		return (false);
	else if (path.has_extension() == false)
		return (false);
	else if ((path.extension() != CGI_EXT_DEFAULT) and
		(path.extension() != CGI_EXT_PY) and
		(path.extension() != CGI_EXT_SH))
		return (false);
	else
		return (true);
}

HTTPexecutor::HTTPexecutor( HTTPexecutor const& other ) noexcept
{
	(void) other;
}

HTTPexecutor& HTTPexecutor::operator=( HTTPexecutor const& other ) noexcept
{
	(void) other;
	return (*this);
}
