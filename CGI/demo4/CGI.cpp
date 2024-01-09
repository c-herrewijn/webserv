#include "CGI.hpp"
#include "HTTPparser.hpp"
#include "Server.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <string.h>

CGI::CGI(
    HTTPrequest &req,
    Server &srv,
    std::string CGIfileName,
    std::string CGIfilePath
)
    : _cgiFileName(CGIfileName),
      _cgiPath(CGIfilePath),
      _CGIEnvArr(this->_createCgiEnv(req, srv)),
      _CgiEnvCStyle(this->_createCgiEnvCStyle())
{}

CGI::~CGI() {
    delete[] this->_CgiEnvCStyle;
}

std::array<std::string, CGI_ENV_SIZE> CGI::_createCgiEnv(HTTPrequest &req, Server &srv)
{
    // split "Host" in addr and port. TODO: error handling via try/catch
    std::istringstream ss(req.headers["Host"]);
    std::string addr;
    std::string port;
    std::getline(ss, addr, ':');
    std::getline(ss, port);

    std::string method;
    if (req.head.method == HTTP_GET) {
        method = "GET";
    } else if (req.head.method == HTTP_POST) {
        method = "POST";
    } else if (req.head.method == HTTP_DELETE) {
        method = "DELETE";
    }

    std::array<std::string, CGI_ENV_SIZE> CGIEnv {
        "AUTH_TYPE=",
        "CONTENT_LENGTH=",
        "CONTENT_TYPE=",
        "GATEWAY_INTERFACE=CGI/1.1", // fixed
        "PATH_INFO=",
        "PATH_TRANSLATED=",
        "QUERY_STRING=", // TODO (part behind '?'-char of the script URI)
        "REMOTE_ADDR=" + addr,
        "REMOTE_HOST=",
        "REMOTE_IDENT=",
        "REMOTE_USER=",
        "REQUEST_METHOD=" + method,
        "SCRIPT_NAME=" + req.head.url.path,
        "SERVER_NAME=" + srv.getNames()[0], // TODO: validations try/catch!
        "SERVER_PORT=" + port,
        "SERVER_PROTOCOL=HTTP/1.1", // fixed
        "SERVER_SOFTWARE=WebServServer", // fixed
        "HTTP_COOKIE=", // TODO
    };
    return CGIEnv;
}

char **CGI::_createCgiEnvCStyle(void)
{
    int i = 0;
    char **CgiEnv = new char*[CGI_ENV_SIZE + 1];
    while (i < CGI_ENV_SIZE)
    {
        CgiEnv[i] = new char[this->_CGIEnvArr[i].length()+1];
        strncpy(CgiEnv[i], this->_CGIEnvArr[i].c_str(), this->_CGIEnvArr[i].length()+1);
        i++;
    }
    CgiEnv[i] = NULL;
    return CgiEnv;
}

std::string CGI::getHTTPResponse()
{
    int p1[2];
	char read_buff[CGI_READ_BUFFER_SIZE];
    bzero(read_buff, CGI_READ_BUFFER_SIZE); // bzero() is not allowed!

    // run cgi, and write result into pipe
	pipe(p1);
	pid_t childPid = fork();
	if (childPid == 0)
	{
	    close(p1[0]);
        dup2(p1[1], STDOUT_FILENO);
        char *argv[2] = {(char*)this->_cgiFileName.c_str(), NULL};

        int res = execve(this->_cgiPath.c_str(), argv, this->_CgiEnvCStyle);
        if (res != 0)
        {
            close(p1[1]);
            std::cerr << "execve error!" << std::endl;
            perror("");
            exit(1); // exit() is not allowed!
        }
	}

    // return cgi response
    int	stat_loc;
    close(p1[1]);
    waitpid(childPid, &stat_loc, 0);
    read(p1[0], read_buff, CGI_READ_BUFFER_SIZE);
    close(p1[0]);
    std::string response = read_buff;
    return response;
}

// disabled default constructor, copy constructor and copy assignment operator
// -----------------------------------------
CGI::CGI() {}

CGI::CGI(const CGI &obj) {
    *this = obj;
}

CGI &CGI::operator=(const CGI &obj) {
    (void)obj;
    return *this;
}
