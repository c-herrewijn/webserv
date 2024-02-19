/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Executor.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:11:07 by fra           #+#    #+#                 */
/*   Updated: 2024/02/17 17:14:07 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Executor.hpp"
#include "CGI.hpp"

Executor::Executor( Server const& config, HTTPrequest& req) noexcept : _configServer(config) , _request(req)
{
    this->_servName = config.getPrimaryName();
}

HTTPresponse	Executor::execRequest( void ) noexcept
{
    int             status = 200;
    HTTPresponse    response;
    std::string     body;

    try
    {
        status = this->_configServer.validateRequest(this->_request);
        if (status != 200)
			throw(ExecException({"request validation failed with code:", std::to_string(status)}, status));
		this->_request.readBody(this->_configServer.getMaxBodySize());		//	<-- depends on the location!
		std::cout << this->_request.toString();
        body = _runHTTPmethod();
    }
    catch(const HTTPexception& e)
    {
        std::cerr << e.what() << '\n';
        status = e.getStatus();
    }
    response = createResponse(status, body);
    return (response);
}

HTTPresponse	Executor::createResponse( int status, std::string const& bodyResp ) noexcept
{
	HTTPresponse response;

    response.parseFromStatic(status, this->_configServer.getPrimaryName(), bodyResp);
	return (response);
}

Server const&		Executor::getHandler( void ) const noexcept
{
    return (this->_configServer);
}

void				Executor::setRequest( HTTPrequest& req) noexcept
{
	this->_request = req;
}

HTTPrequest const&	Executor::getRequest( void ) const noexcept
{
	return(this->_request);
}

std::string	Executor::_runHTTPmethod( void )
{
    std::string	body;

	if (this->_request.isCGI() == true)
    {
        // CGI
		CGI CGIrequest(this->_request, this->_configServer);
        body = CGIrequest.getHTMLBody();
	}
	else {
		// non-CGI
		switch (this->_request.getMethod())
		{
			case HTTP_GET:
			{
				body = _execGET();
				break ;
			}
			case HTTP_POST:
			{
				body = _execPOST();
				break ;
			}
			case HTTP_DELETE:
			{
				body = _execDELETE();
				break ;
			}
		}
	}
	return (body);
}

std::string	Executor::_execGET( void )
{
	std::string pathReq = this->_request.getPath();
	std::string htmlBody;

	if (pathReq == "/")
		htmlBody = _readContent("./var/www/test.html");
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

std::string	Executor::_execPOST( void )
{
	std::string pathReq = this->_request.getPath();
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

std::string	Executor::_execDELETE( void )
{
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
