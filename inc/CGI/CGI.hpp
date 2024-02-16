#ifndef CGI_HPP
# define CGI_HPP

# define CGI_ENV_SIZE 19
# define CGI_READ_BUFFER_SIZE 10000

# include "Server.hpp"

# include <array>
# include <string>
# include <strings.h> // bzero()
# include <unistd.h>  // pipe(), fork()
# include <sys/wait.h>  // waitpid()

class  CGI {
public:
    CGI(const HTTPrequest &req, const Server &srv);
    ~CGI();
    std::string getHTMLBody();
    int *getuploadPipe();
    int *getResponsePipe();

private:
    const HTTPrequest &_req;
    const Server &_srv;
    std::array<std::string, CGI_ENV_SIZE> _CGIEnvArr;
    char *const *_CgiEnvCStyle;
    int _uploadPipe[2];
    int _responsePipe[2];

    std::array<std::string, CGI_ENV_SIZE> _createCgiEnv(const HTTPrequest &req, const Server &srv);
    char **_createCgiEnvCStyle();
};

#endif
