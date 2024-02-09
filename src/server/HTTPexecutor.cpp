/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPexecutor.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:11:07 by fra           #+#    #+#                 */
/*   Updated: 2024/02/09 14:53:56 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPexecutor.hpp"

// const std::map<HTTPmethod, std::function<std::string(HTTPrequest&, int&)> > HTTPexecutor::_methods = 
// {
// 	{HTTP_GET, _execGET},
// 	{HTTP_POST, _execPOST},
// 	{HTTP_DELETE, _execDELETE},
// };

// int	HTTPexecutor::execRequest(HTTPrequest& req, std::string& body)
// {
// 	int reqStatus = -1;

// 	try
// 	{
// 		body = HTTPexecutor::_methods.at(req.head.method)(req, reqStatus);
// 	}
// 	catch(const std::out_of_range& e) {
// 		std::cerr << "unsupported HTTP method\n";
// 		reqStatus = 501;
// 	}
// 	return(reqStatus);
// }

// std::string	HTTPexecutor::_execGET(HTTPrequest& req, int& status)
// {
// 	std::string pathReq = "var/www/test.html"; //req.head.url.path;
// 	std::string htmlBody;
// 	(void) req;

// 	try
// 	{
// 		status = _checkPath(pathReq, R_OK);
// 		if (status != 200)
// 			return ("");
// 		else if (_isCGI(pathReq))
// 		{
// 			std::cout << "CGI\n";
// 			// fork and do CGI
// 		}
// 		else
// 		{
// 			htmlBody = _readContent(pathReq);
// 		}
// 		status = 200;
// 	}
// 	catch (ExecException const& e) {
// 		std::cerr << e.what() << '\n';
// 		status = 500;
// 	}
// 	return (htmlBody);
// }

// std::string	HTTPexecutor::_execPOST(HTTPrequest& req, int& status)
// {
// 	std::string pathReq = req.head.url.path;
// 	std::string htmlBody;

// 	try
// 	{
// 		status = _checkPath(pathReq, W_OK);
// 		if (status != 200)
// 			return ("");
// 		else if (_isCGI(pathReq))
// 		{
// 			std::cout << "CGI\n";
// 			// fork and do CGI
// 		}
// 		else
// 		{
// 			htmlBody = "";
// 		}
// 		status = 200;
// 	}
// 	catch (ExecException const& e) {
// 		std::cerr << e.what() << '\n';
// 		status = 500;
// 	}
// 	return (htmlBody);
// }

// std::string	HTTPexecutor::_execDELETE(HTTPrequest& req, int& status)
// {
// 	(void) req;
// 	status = 200;
// 	return ("");
// }

// std::string	HTTPexecutor::_readContent(std::string const& pathReq)
// {
// 	std::fstream	fd(pathReq.c_str());
// 	std::string		body, line;

// 	if (!fd.is_open())
// 		throw(ExecException({"error opening file", pathReq}));	// NB not an exception! has to be the correspondant to 40X error code
// 	while (std::getline(fd, line))
// 		body += line + std::string("\n");
// 	fd.close();
// 	return (body);
// }

// int	HTTPexecutor::_checkPath(std::string const& path, int action)
// {
// 	if ((action != F_OK) and (action != R_OK) and (action != W_OK) and (action != X_OK))
// 		throw(ExecException({"Unknown access mode:", std::to_string(action)}));
// 	if (access(path.c_str(), F_OK) != 0)
// 		return (404);
// 	else if (access(path.c_str(), action) != 0)
// 		return (403);
// 	else
// 		return (200);
	
// }

// bool	HTTPexecutor::_isCGI(std::string const& path)
// {
// 	// what about checks for the directory?
// 	size_t		dotPos = path.rfind('.');
// 	std::string	ext;
// 	if (dotPos == std::string::npos)
// 		return (false);
// 	ext = path.substr(dotPos);
// 	return ((ext == CGI_EXT_PY) or (ext == CGI_EXT_SH));
// }
