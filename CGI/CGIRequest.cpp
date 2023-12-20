#include "CGIRequest.hpp"
#include <string>
#include <iostream>

CGIRequest::CGIRequest()
    : _cgiFileName("cgi.sh"),
      _cgiPath("./cgi.sh"),
      _CGIEnvArr(this->createCgiEnv()),
      _CgiEnvCStyle(this->createCgiEnvCStyle())
{}

CGIRequest::~CGIRequest() {}

void CGIRequest::printEnv()
{
    std::cout << "debug: printing env" << std::endl;
    std::cout << this->_CGIEnvArr[0] << std::endl;
    std::cout << this->_CGIEnvArr[1] << std::endl;
    std::cout << this->_CGIEnvArr[16] << std::endl;
    std::cout << this->_CgiEnvCStyle[0] << std::endl;
    std::cout << this->_CgiEnvCStyle[1] << std::endl;
    std::cout << this->_CgiEnvCStyle[16] << std::endl;

}

std::array<std::string, CGI_ENV_SIZE> CGIRequest::createCgiEnv()
{
    std::array<std::string, CGI_ENV_SIZE> CGIEnv {
        "AUTH_TYPE=",
        "CONTENT_LENGTH=",
        "CONTENT_TYPE=",
        "GATEWAY_INTERFACE=1.1",
        "PATH_INFO=",
        "PATH_TRANSLATED=",
        "QUERY_STRING=",
        "REMOTE_ADDR=",
        "REMOTE_HOST=",
        "REMOTE_IDENT=",
        "REMOTE_USER=",
        "REQUEST_METHOD=",
        "SCRIPT_NAME=",
        "SERVER_NAME=MyServer",
        "SERVER_PORT=",
        "SERVER_PROTOCOL=",
        "SERVER_SOFTWARE=",
    };
    return CGIEnv;
}

char *const *CGIRequest::createCgiEnvCStyle()
{
    char *const *CgiEnv = new char*[CGI_ENV_SIZE + 1] {
        (char *)this->_CGIEnvArr[0].c_str(),
        (char *)this->_CGIEnvArr[1].c_str(),
        (char *)this->_CGIEnvArr[2].c_str(),
        (char *)this->_CGIEnvArr[3].c_str(),
        (char *)this->_CGIEnvArr[4].c_str(),
        (char *)this->_CGIEnvArr[5].c_str(),
        (char *)this->_CGIEnvArr[6].c_str(),
        (char *)this->_CGIEnvArr[7].c_str(),
        (char *)this->_CGIEnvArr[8].c_str(),
        (char *)this->_CGIEnvArr[9].c_str(),
        (char *)this->_CGIEnvArr[10].c_str(),
        (char *)this->_CGIEnvArr[11].c_str(),
        (char *)this->_CGIEnvArr[12].c_str(),
        (char *)this->_CGIEnvArr[13].c_str(),
        (char *)this->_CGIEnvArr[14].c_str(),
        (char *)this->_CGIEnvArr[15].c_str(),
        (char *)this->_CGIEnvArr[16].c_str(),
        (char *)NULL
    };
    return CgiEnv;
}

std::string CGIRequest::runCgi()
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



// disabled copy constructor and copy assignment operator
// -----------------------------------------
CGIRequest::CGIRequest(const CGIRequest &obj) {
    *this = obj;
}

CGIRequest &CGIRequest::operator=(const CGIRequest &obj) {
    (void)obj;
    return *this;
}
