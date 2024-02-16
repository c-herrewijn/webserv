/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Executor.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:11:07 by fra           #+#    #+#                 */
/*   Updated: 2024/02/16 10:58:05 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Executor.hpp"

HTTPresponse	Executor::execRequest(HTTPrequest& req, Server const& handler ) noexcept
{
    int             status = 200;
    HTTPresponse    response;
    std::string     body;

    try
    {
        status = handler.validateRequest(req);
        if (status != 200)
			throw(ExecException({"request validation failed with code:", std::to_string(status)}, status));
		req.readRemainingBody(handler.getMaxBodySize());		//	<-- depends on the location!
		req.parseBody();
        std::cout << req.toString();
        body = _runMethod(req);
    }
    catch(const HTTPexception& e)
    {
        std::cerr << e.what() << '\n';
        status = e.getStatus();
    }
    response = createResponse(status, handler.getPrimaryName(), body);
    return (response);
}

HTTPresponse	Executor::createResponse( int status, std::string const& servName, std::string const& bodyResp ) noexcept
{
	HTTPresponse response;

    response.buildResponse(status, servName, bodyResp);
	return (response);
}

std::string	Executor::_runMethod(HTTPrequest const& req)
{
    std::string	body;

	switch (req.getMethod())
	{
		case HTTP_GET:
		{
			body = _execGET(req);
			break ;
		}
		case HTTP_POST:
		{
			body = _execPOST(req);
			break ;
		}
		case HTTP_DELETE:
		{
			body = _execDELETE(req);
			break ;
		}
	}
	return (body);
}

std::string	Executor::_execGET(HTTPrequest const& req)
{
	std::string pathReq = req.getPath();
	std::string htmlBody;

	// try
	// {
	// 	status = _checkPath(pathReq, R_OK);
	// 	if (status != 200)
	// 		return ("");
	// 	else if (_isCGI(pathReq))
	// 	{
	// 		std::cout << "CGI\n";
	// 		// fork and do CGI
	// 	}
	// 	else
	// 	{
			// htmlBody = _readContent(pathReq);
	// 	}
	// 	status = 200;
	// }
	// catch (ExecException const& e) {
	// 	std::cerr << e.what() << '\n';
	// 	status = 500;
	// }
	return (htmlBody);
}

std::string	Executor::_execPOST(HTTPrequest const& req)
{
	std::string pathReq = req.getPath();
	std::string htmlBody;

	// try
	// {
	// 	status = _checkPath(pathReq, W_OK);
	// 	if (status != 200)
	// 		return ("");
	// 	else if (_isCGI(pathReq))
	// 	{
	// 		std::cout << "CGI\n";
	// 		// fork and do CGI
	// 	}
	// 	else
	// 	{
	// 		htmlBody = "";
	// 	}
	// 	status = 200;
	// }
	// catch (ExecException const& e) {
	// 	std::cerr << e.what() << '\n';
	// 	status = 500;
	// }
	return (htmlBody);
}

std::string	Executor::_execDELETE(HTTPrequest const& req)
{
	(void) req;
	return ("");
}

std::string	Executor::_readContent(std::string const& pathReq)
{
	std::fstream	fd(pathReq.c_str());
	std::string		body, line;

	if (!fd.is_open())
		throw(ExecException({"error opening file", pathReq}, 500));	// NB not an exception! has to be the correspondant to 40X error code
	while (std::getline(fd, line))
		body += line + std::string("\n");
	fd.close();
	return (body);
}
