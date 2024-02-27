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

RequestExecutor::RequestExecutor( int socket )
{
	if (socket == -1)
		throw(HTTPexception({"invalid socket"}, 500));
	this->_socket = socket;
	this->_configServer = nullptr;
	this->_request = nullptr;
};


// NB: handle it better in case of error
// NB: incase of errors in CGI(python) they has to treated accordingly,
//	otherwise the python trace will be written in the body of the CGI response
HTTPresponse	RequestExecutor::execRequest( void ) noexcept
{
    int             status = 200;
    HTTPresponse    response;
    std::string     responseBody;

    try
    {
        status = this->_configServer->validateRequest(*(this->_request));
        if (status != 200)
			throw(ExecException({"request validation failed with code:", std::to_string(status)}, status));
		this->_request->parseBody(this->_socket, 1000000);	// NB: this needs to be dynamic depending on the location
		responseBody = _runHTTPmethod();
    }
    catch(const HTTPexception& e)
    {
        std::cerr << e.what() << '\n';
        status = e.getStatus();
    }
    response = createResponse(status, responseBody);
    return (response);
}

// NB: handle it better in case of error
HTTPresponse	RequestExecutor::createResponse( int status, std::string const& bodyResp ) noexcept
{
	HTTPresponse response;

	if (this->_request->isCGI())
	{
		try {
			response.parseFromCGI(bodyResp);
		}
		catch(const ResponseException& e) {
			std::cerr << e.what() << '\n';
			response.parseFromStatic(e.getStatus(), this->_configServer->getPrimaryName(), "");
		}
	}
	else
	    response.parseFromStatic(status, this->_configServer->getPrimaryName(), bodyResp);
	return (response);
}

int		RequestExecutor::getSocket( void ) const noexcept
{
	return (this->_socket);
}

HTTPrequest const&	RequestExecutor::getRequest( void ) const noexcept
{
	return(*(this->_request));
}

ConfigServer const&		RequestExecutor::getConfigServer( void ) const noexcept
{
    return (*(this->_configServer));
}

void	RequestExecutor::setRequest( HTTPrequest *req) noexcept
{
	this->_request = req;
}

void 	RequestExecutor::setConfigServer(ConfigServer const* config) noexcept
{
	this->_configServer = config;
	this->_servName = config->getPrimaryName();
}

std::string	RequestExecutor::_runHTTPmethod( void )
{
    std::string	responseBody;

	if (this->_request->isCGI() == true)
    {
        // CGI
		CGI CGIrequest(*(this->_request), *(this->_configServer));
        responseBody = CGIrequest.getHTMLBody();
	}
	else {
		// non-CGI
		switch (this->_request->getMethod())
		{
			case HTTP_GET:
			{
				responseBody = _execGET();
				break ;
			}
			case HTTP_POST:
			{
				responseBody = _execPOST();
				break ;
			}
			case HTTP_DELETE:
			{
				responseBody = _execDELETE();
				break ;
			}
		}
	}
	return (responseBody);
}

std::string	RequestExecutor::_execGET( void )
{
	std::string pathReq = this->_request->getPath();
	std::string htmlBody;

	if (pathReq == "/")
		htmlBody = _readContent("./var/www/test.html");
	return (htmlBody);
}

std::string	RequestExecutor::_execPOST( void )
{
	std::string pathReq = this->_request->getPath();
	std::string htmlBody;

	return (htmlBody);
}

std::string	RequestExecutor::_execDELETE( void )
{
	return ("");
}

std::string	RequestExecutor::_readContent(std::string const& pathReq)
{
	std::fstream	fd(pathReq.c_str());
	std::string		fileContent, line;

	if (!fd.is_open())
		throw(ExecException({"error opening file", pathReq}, 500));	// NB not an exception! has to be the correspondant to 40X error code
	while (std::getline(fd, line))
		fileContent += line + std::string("\n");
	fd.close();
	return (fileContent);
}
