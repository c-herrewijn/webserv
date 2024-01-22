/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPexecutor.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:11:07 by fra           #+#    #+#                 */
/*   Updated: 2024/01/22 18:41:20 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPexecutor.hpp"

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
		throw(ExecException({"unsupported HTTP method"}));		// NB: that should be move inside the parser
	}
	return(reqStatus);
}

std::string	HTTPexecutor::_execGET(HTTPrequest& req, int& status)
{
	std::string pathReq = req.head.url.path;
	std::string htmlBody;

	try
	{
		_checkPath(pathReq);
		if (_isCGI(pathReq))
		{
			std::cout << "CGI\n";
			// fork and do CGI
		}
		else
		{
			htmlBody = _readContent(pathReq);
		}
		status = 200;
	}
	catch (ExecException const& e) {
		std::cerr << e.what() << '\n';
		status = 403; // <-- e.g. !
	}
	return (htmlBody);
}

std::string	HTTPexecutor::_execPOST(HTTPrequest& req, int& status)
{
	// if (_isCGI(req.head.url.path) == true)
	// 	std::cout << "CGI\n";
	// else
	// 	std::cout << "not CGI\n";
	(void) req;
	status = 200;
	return ("");
}

std::string	HTTPexecutor::_execDELETE(HTTPrequest& req, int& status)
{
	(void) req;
	status = 200;
	return ("");
}

std::string	HTTPexecutor::_readContent(std::string const& pathReq)
{
	std::fstream	fd(pathReq.c_str());
	std::string		body, line;

	if (!fd.is_open())
		throw(ExecException({"error opening file", pathReq.c_str()}));	// NB not an exceptio! has to be the correspondante 40X error code
	while (std::getline(fd, line))
		body += line + std::string("\n");
	fd.close();
	return (body);
}

void	HTTPexecutor::_checkPath(std::string const& path)
{
	if (access(path.c_str(), R_OK) != 0)
		throw(ExecException({"permission error"}));	// NB not an exceptio! has to be the correspondante 40X error code
}

bool	HTTPexecutor::_isCGI(std::string const& path)
{
	// what about checks for the directory?
	size_t		dotPos = path.rfind('.');
	std::string	ext;
	if (dotPos == std::string::npos)
		return (false);
	ext = path.substr(dotPos);
	return ((ext == CGI_EXT_PY) or (ext == CGI_EXT_SH));
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
