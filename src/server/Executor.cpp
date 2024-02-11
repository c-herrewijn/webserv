/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Executor.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:11:07 by fra           #+#    #+#                 */
/*   Updated: 2024/02/11 03:51:58 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Executor.hpp"

// const std::map<HTTPmethod, std::function<std::string(HTTPrequest&, int&)> > HTTPexecutor::_methods = 
// {
// 	{HTTP_GET, _execGET},
// 	{HTTP_POST, _execPOST},
// 	{HTTP_DELETE, _execDELETE},
// };

Executor::Executor( void ) noexcept : _maxLenBody(-1) {}

Executor::Executor( Server const& server ) noexcept : _handler(server)
{
    this->_servName = server.getPrimaryName();
    this->_maxLenBody = this->_handler.getParams().getMaxSize().first + HTTP_TERM.size();
}

int		Executor::storeRemainingBody( HTTPrequest& request, int socket) const
{
    ssize_t     lenToRead = this->_maxLenBody - request.getTmpBody().size();
    ssize_t     readChar = -1;
    int         status = 200;
    char        *buffer = nullptr;
    std::string body;

    if (lenToRead < 0)
        throw(RequestException({"body length is longer than maximum allowed"}));
    buffer = new char[lenToRead + 1];
    bzero(buffer, lenToRead + 1);
    readChar = recv(socket, buffer, lenToRead, 0);
    body = buffer;
    delete [] buffer;
    if (readChar == -1)
        throw(ServerException({"socket not available"}));
    request.parseBody(body);
    return (status);
}

HTTPresponse	Executor::execRequest(HTTPrequest& req ) const
{
    HTTPresponse response;
    int exitStatus = 200;
    std::string body;
	if (req.isReady() == false)
		throw(ExecException({"request is not ready to be executed"}));
	// do the checks
    // exec HTTP method
    response = createResponse(exitStatus, body);
    return (response);
}

HTTPresponse	Executor::createResponse( int status, std::string bodyResp ) const
{
	HTTPresponse response;

    response.buildResponse(status, this->_servName, bodyResp);
	return (response);
}

void				Executor::setHandler( Server const& handler)
{
    this->_handler = handler;
}

Server const&		Executor::getHandler( void ) const 
{
    return (this->_handler);
}

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
