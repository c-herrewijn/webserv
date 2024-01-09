#ifndef CGI_HPP
# define CGI_HPP

# define CGI_ENV_SIZE 18
# define CGI_READ_BUFFER_SIZE 10000

# include "HTTPparser.hpp"
# include "Server.hpp"

# include <array>
# include <string>
# include <strings.h> // bzero()
# include <unistd.h>  // pipe(), fork()
# include <sys/wait.h>  // waitpid()

class  CGI {
public:
    CGI(HTTPrequest &req, Server &srv, std::string CGIfileName, std::string CGIfilePath, std::string serverName);
    ~CGI();
    std::string getHTTPResponse();

private:
    const std::string _cgiFileName;
    const std::string _cgiPath;
    std::array<std::string, CGI_ENV_SIZE> _CGIEnvArr;
    char *const *_CgiEnvCStyle;

    std::array<std::string, CGI_ENV_SIZE> createCgiEnv(HTTPrequest &req, Server &srv);
    char **createCgiEnvCStyle();

    // disabled default constructor, copy constructor, and copy assignment operator
    CGI();
    CGI(const CGI &obj);
    CGI &operator=(const CGI &obj);
};

#endif
