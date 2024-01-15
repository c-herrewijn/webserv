/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPexecutor.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:11:07 by fra           #+#    #+#                 */
/*   Updated: 2024/01/15 19:59:07 by fra           ########   odam.nl         */
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

const std::map<HTTPmethod, std::function<std::string(HTTPrequest&, int&)> > HTTPexecutor::_methods = 
{
	{HTTP_GET, _execGET},
	{HTTP_POST, _execPOST},
	{HTTP_DELETE, _execDELETE},
};

int	HTTPexecutor::execRequest(HTTPrequest& req, std::string& body)
{
	int reqStatus = -1;

	try
	{
		body = HTTPexecutor::_methods.at(req.head.method)(req, reqStatus);
	}
	catch(const std::out_of_range& e) {
		throw(ExecException({"unsupported HTTP method"}));
	}
	return(reqStatus);
}

std::string	HTTPexecutor::_execGET(HTTPrequest& req, int& status)
{
	std::filesystem::path pathReq = req.head.url.path;
	std::string htmlBody;

	_checkPath(pathReq);
	if (_isCGI(pathReq) == true)
	{
		std::cout << "CGI\n";
		// do CGI stuff ....
	}
	else
	{
		try
		{
			htmlBody = _readContent(pathReq);
		}
		catch(const ExecException& e) {
			std::cerr << e.what() << '\n';
			// handle error, i.e. update status response
		}
		
	}
	status = 200;
	return (htmlBody);
}

std::string	HTTPexecutor::_execPOST(HTTPrequest& req, int& status)
{
	if (_isCGI(req.head.url.path) == true)
		std::cout << "CGI\n";
	else
		std::cout << "not CGI\n";
	std::string body;
	status = 200;
	return ("");
}

std::string	HTTPexecutor::_execDELETE(HTTPrequest& req, int& status)
{
	(void) req;
	status = 200;
	return ("");
}

std::string	HTTPexecutor::_readContent(std::filesystem::path& pathReq)
{
	std::ifstream	fd(pathReq.string().c_str());
	std::string		body, line;

	if (!fd.is_open())
		throw(ExecException({"error opening file", pathReq.string().c_str()}));
	while (std::getline(fd, line))
		body += line + std::string("\n");
	fd.close();
	return (body);
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

	// there may be other CGI paths if the config says so
	baseDir = path.parent_path();
	if (baseDir.is_absolute())
	{
		if (std::filesystem::absolute(CGI_DIR) != baseDir)
			return (false);
	}
	else
	{
		// relative paths must be handled!
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
