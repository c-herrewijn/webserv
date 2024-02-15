/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Executor.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:11:07 by fra           #+#    #+#                 */
/*   Updated: 2024/02/15 17:59:14 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Executor.hpp"

Executor::Executor( void ) noexcept : _maxLenBody(-1) {}

Executor::Executor( Server const& server ) noexcept : _handler(server)
{
    this->_servName = server.getPrimaryName();
    this->_maxLenBody = this->_handler.getParams().getMaxSize() + HTTP_TERM.size();
}

HTTPresponse	Executor::execRequest(HTTPrequest& req ) const noexcept
{
    int             status = 200;
    HTTPresponse    response;
    std::string     reqMethod, body;

    try
    {
        status = this->_handler.validateRequest(req);
        if (status != 200)
        	throw(ExecException({"validation failed with code:", std::to_string(status)}, status));
        else if (req.isReady() == false)
		    throw(ExecException({"request is not ready to be executed"}, 500));
        reqMethod = req.getMethod();
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
    }
    catch(const ExecException& e)
    {
        std::cerr << e.what() << '\n';
        status = e.getStatus();
    }
    response = createResponse(status, body);
    return (response);
}

HTTPresponse	Executor::createResponse( int status, std::string bodyResp ) const noexcept
{
	HTTPresponse response;

    response.buildResponse(status, this->_servName, bodyResp);
	return (response);
}

void				Executor::setHandler( Server const& handler) noexcept
{
    this->_handler = handler;
    this->_servName = handler.getPrimaryName();
}

Server const&		Executor::getHandler( void ) const noexcept
{
    return (this->_handler);
}

std::string	Executor::_execGET(HTTPrequest& req) const
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
			htmlBody = _readContent(pathReq);
	// 	}
	// 	status = 200;
	// }
	// catch (ExecException const& e) {
	// 	std::cerr << e.what() << '\n';
	// 	status = 500;
	// }
	return (htmlBody);
}

std::string	Executor::_execPOST(HTTPrequest& req) const
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

std::string	Executor::_execDELETE(HTTPrequest& req) const
{
	(void) req;
	return ("");
}

std::string	Executor::_readContent(std::string const& pathReq) const
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
