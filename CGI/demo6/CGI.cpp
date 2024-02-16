#include "CGI.hpp"
#include "HTTPparser.hpp"
#include "Server.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <string.h>

CGI::CGI(
    HTTPrequest &req,
    Server &srv
)
    : _req(req),
      _srv(srv),
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
        // "CONTENT_LENGTH=" + std::to_string(this->_req.body.length()),
        "CONTENT_LENGTH=229", // DUMMY value based on test string
        "CONTENT_TYPE=multipart/form-data; boundary=---------------------------47207745632788886342367179802", // DUMMY
        // "CONTENT_TYPE=multipart/form-data", // NOT working
        "GATEWAY_INTERFACE=CGI/1.1", // fixed
        "PATH_INFO=",
        "PATH_TRANSLATED=",
        "QUERY_STRING=" + req.head.url.queryRaw,
        "REMOTE_ADDR=" + addr,
        "REMOTE_HOST=",
        "REMOTE_IDENT=",
        "REMOTE_USER=",
        "REQUEST_METHOD=" + method,
        "SCRIPT_NAME=" + req.head.url.path.substr(req.head.url.path.find_last_of("/\\") + 1), // script name, e.g. myScript.cgi
        "SCRIPT_FILENAME=" + req.head.url.path, // script path relative to document root, e.g. /cgi-bin/myScript.cgi
        "SERVER_NAME=" + srv.getNames()[0], // TODO: validations try/catch!
        "SERVER_PORT=" + port,
        "SERVER_PROTOCOL=HTTP/1.1", // fixed
        "SERVER_SOFTWARE=WebServServer/1.0", // fixed
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
        CgiEnv[i] = (char *)this->_CGIEnvArr[i].c_str();
        i++;
    }
    CgiEnv[i] = NULL;
    return CgiEnv;
}

std::string CGI::getHTTPResponse()
{
    int p1[2]; // pipe where CGI writes response
    int p2[2]; // pipe where CGI reads body
	char read_buff[CGI_READ_BUFFER_SIZE];
    bzero(read_buff, CGI_READ_BUFFER_SIZE); // bzero() is not allowed!

    // run cgi, and write result into pipe
	pipe(p1);
	pipe(p2);
	pid_t childPid = fork();
	if (childPid == 0)
	{
	    close(p1[0]);
        dup2(p1[1], STDOUT_FILENO); // write to pipe
        close(p2[1]);
        dup2(p2[0], STDIN_FILENO); // read from pipe
        std::string CGIfilePath = _srv.getParams().getRoot() + _req.head.url.path;
        std::string CGIfileName = CGIfilePath.substr(CGIfilePath.rfind("/")+1); // fully stripped, only used for execve
        char *argv[2] = {(char*)CGIfileName.c_str(), NULL};
        int res = execve(CGIfilePath.c_str(), argv, this->_CgiEnvCStyle);
        if (res != 0)
        {
            close(p1[1]);
            std::cerr << "Error in running CGI script!" << std::endl;
            perror("");
            exit(1); // exit() is not allowed!
        }
	}
    // write body into pipe
    close(p2[0]);
    std::cerr << C_RED << "parsed body: " << this->_req.body << C_RESET << std::endl;
    std::string testStr =
        "-----------------------------47207745632788886342367179802\r\n"
        "Content-Disposition: form-data; name=\"filename\"; filename=\"test1.txt\"\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "aaaaa\r\n"
        "\r\n"
        "-----------------------------47207745632788886342367179802--\r\n";

    std::cerr << "test string length: " << testStr.length() << std::endl;

    // write(p2[1], this->_req.body.c_str(), this->_req.body.length());
    write(p2[1], testStr.c_str(), testStr.length());
    close(p2[1]);

    // return cgi response
    int	stat_loc;
    close(p1[1]);
    waitpid(childPid, &stat_loc, 0);
    read(p1[0], read_buff, CGI_READ_BUFFER_SIZE);
    close(p1[0]);
    std::string response = read_buff;
    return response;
}

int *CGI::getuploadPipe(){
    return this->_uploadPipe;
}

int *CGI::getResponsePipe() {
    return this->_responsePipe;
}
