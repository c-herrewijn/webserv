/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestExecutor.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:11:07 by fra           #+#    #+#                 */
/*   Updated: 2024/02/20 16:02:27 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "RequestExecutor.hpp"
#include "CGI.hpp"

// RequestExecutor::RequestExecutor( ConfigServer const& config, HTTPrequest& req) noexcept : _configServer(config) , _request(req)
// {
//     this->_servName = config.getPrimaryName();
// }

RequestExecutor::RequestExecutor( int socket )
{
	if (socket == -1)
		throw(HTTPexception({"invalid socket"}, 500));
	this->_socket = socket;
	this->_configServer = nullptr;
	this->_request = nullptr;
};

void 	RequestExecutor::setConfigServer(ConfigServer const* config) noexcept
{
	this->_configServer = config;
	this->_servName = config->getPrimaryName();
}

HTTPresponse	RequestExecutor::execRequest( void ) noexcept
{
    int             status = 200;
    HTTPresponse    response;
    std::string     body;

    try
    {
        status = this->_configServer->validateRequest(*(this->_request));
        if (status != 200)
			throw(ExecException({"request validation failed with code:", std::to_string(status)}, status));
		this->_request->parseBody(this->_socket, 1000000);	// NB: this needs to be dynamic depending on the location
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

HTTPresponse	RequestExecutor::createResponse( int status, std::string const& bodyResp ) noexcept
{
	HTTPresponse response;

    response.parseFromStatic(status, this->_configServer->getPrimaryName(), bodyResp);
	return (response);
}

ConfigServer const&		RequestExecutor::getHandler( void ) const noexcept
{
    return (*(this->_configServer));
}

void				RequestExecutor::setRequest( HTTPrequest *req) noexcept
{
	this->_request = req;
}

HTTPrequest const&	RequestExecutor::getRequest( void ) const noexcept
{
	return(*(this->_request));
}

int		RequestExecutor::getSocket( void ) const noexcept
{
	return (this->_socket);
}

std::string	RequestExecutor::_runHTTPmethod( void )
{
    std::string	body;

	if (this->_request->isCGI() == true)
    {
        // CGI
		CGI CGIrequest(*(this->_request), *(this->_configServer));
        body = CGIrequest.getHTMLBody();
	}
	else {
		// non-CGI
		switch (this->_request->getMethod())
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

std::string	RequestExecutor::_execGET( void )
{
	std::string pathReq = this->_request->getPath();
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

std::string	RequestExecutor::_execPOST( void )
{
	std::string pathReq = this->_request->getPath();
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

std::string	RequestExecutor::_execDELETE( void )
{
	return ("");
}

std::string	RequestExecutor::_readContent(std::string const& pathReq)
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
