#ifndef CGI_HPP
# define CGI_HPP

# define CGI_ENV_SIZE 18
# define CGI_READ_BUFFER_SIZE 10000

# include "HTTPparser.hpp"
# include "ConfigServer.hpp"

# include <array>
# include <string>
# include <strings.h> // bzero()
# include <unistd.h>  // pipe(), fork()
# include <sys/wait.h>  // waitpid()

class  CGI {
public:
    CGI(HTTPrequest &req, ConfigServer &srv);
    ~CGI();
    std::string getHTTPResponse();

private:
    HTTPrequest &_req;
    ConfigServer &_srv;
    std::array<std::string, CGI_ENV_SIZE> _CGIEnvArr;
    char *const *_CgiEnvCStyle;

    std::array<std::string, CGI_ENV_SIZE> _createCgiEnv(HTTPrequest &req, ConfigServer &srv);
    char **_createCgiEnvCStyle();
};

#endif
