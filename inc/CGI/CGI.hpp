#ifndef CGI_HPP
# define CGI_HPP

# define CGI_ENV_SIZE 19
# define CGI_READ_BUFFER_SIZE 10000
# define ROOT std::string("/home/fra/Codam/webserv/var/www")

# include <array>
# include <string>
# include <strings.h> // bzero()
# include <unistd.h>  // pipe(), fork()
# include <sys/wait.h>  // waitpid()

#include "HTTPrequest.hpp"

class  CGI {
public:
    CGI(const HTTPrequest &req);
    ~CGI();
    void run();
    int *getuploadPipe();
    int *getResponsePipe();
    int getRequestSocket();
    std::string getResponse();
    void appendResponse(std::string additionalResponsePart);

private:
    const HTTPrequest &_req;
    std::array<std::string, CGI_ENV_SIZE> _CGIEnvArr;
    char *const *_CgiEnvCStyle;
    int _uploadPipe[2];
    int _responsePipe[2];
    std::string _response;

    std::array<std::string, CGI_ENV_SIZE> _createCgiEnv(const HTTPrequest &req);
    char **_createCgiEnvCStyle();
};

#endif
