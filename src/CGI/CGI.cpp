#include "CGI.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <string.h>

CGI::CGI(const HTTPrequest &req)
    : _req(req),
      _CGIEnvArr(this->_createCgiEnv(req)),
      _CgiEnvCStyle(this->_createCgiEnvCStyle())
{
    pipe(_uploadPipe);
    pipe(_responsePipe);
}

CGI::~CGI() {
    delete[] this->_CgiEnvCStyle;
}

std::array<std::string, CGI_ENV_SIZE> CGI::_createCgiEnv(const HTTPrequest &req)
{
    // split "Host" in addr and port. TODO: error handling via try/catch
    std::istringstream ss(req.getHost());
    std::string addr;
    std::string port;
    std::getline(ss, addr, ':');
    std::getline(ss, port);

    std::string method = req.getStrMethod();

    std::array<std::string, CGI_ENV_SIZE> CGIEnv {
        "AUTH_TYPE=",
        "CONTENT_LENGTH=" + std::to_string(this->_req.getBody().length()),
        "CONTENT_TYPE=multipart/form-data; boundary=" + this->_req.getContentTypeBoundary(),
        "GATEWAY_INTERFACE=CGI/1.1", // fixed
        "PATH_INFO=",
        "PATH_TRANSLATED=",
        "QUERY_STRING=" + req.getQueryRaw(),
        "REMOTE_ADDR=" + addr,
        "REMOTE_HOST=",
        "REMOTE_IDENT=",
        "REMOTE_USER=",
        "REQUEST_METHOD=" + method,
        "SCRIPT_NAME=" + req.getPath().substr(req.getPath().find_last_of("/\\") + 1), // script name, e.g. myScript.cgi
        "SCRIPT_FILENAME=" + req.getPath(), // script path relative to document root, e.g. /cgi-bin/myScript.cgi
        "SERVER_NAME=" + req.getConfigServer().getPrimaryName(),
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

void CGI::run()
{
    pid_t childPid = fork();
    if (childPid == 0) {
        close(this->_responsePipe[0]);
        dup2(this->_responsePipe[1], STDOUT_FILENO); // write to pipe
        close(this->_uploadPipe[1]);
        dup2(this->_uploadPipe[0], STDIN_FILENO); // read from pipe

        std::string CGIfilePath = _req.getConfigServer().getParams().getRoot() + _req.getPath();
        std::string CGIfileName = CGIfilePath.substr(CGIfilePath.rfind("/")+1); // fully stripped, only used for execve
        char *argv[2] = {(char*)CGIfileName.c_str(), NULL};
        int res = execve(CGIfilePath.c_str(), argv, this->_CgiEnvCStyle);
        if (res != 0)
        {
            close(this->_responsePipe[1]);
            std::cerr << "Error in running CGI script!" << std::endl;
            std::cerr << "path: " << CGIfilePath.c_str() << std::endl;
            perror("");
            exit(1); // TODO: exit() is not allowed!
        }
    }
}

int CGI::getRequestSocket() {
    return this->_req.getSocket();
}

int *CGI::getuploadPipe(){
    return this->_uploadPipe;
}

int *CGI::getResponsePipe() {
    return this->_responsePipe;
}

std::string CGI::getResponse() {
    return this->_response;
}

void CGI::appendResponse(std::string additionalResponsePart) {
    this->_response = this->_response + additionalResponsePart;
}
