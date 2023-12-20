#ifndef CGIREQUEST_H
# define CGIREQUEST_H

# define CGI_ENV_SIZE 17
# define CGI_READ_BUFFER_SIZE 10000

# include <array>
# include <string>
# include <strings.h> // bzero()
# include <unistd.h>  // pipe(), fork()
# include <sys/wait.h>  // waitpid()

class  CGIRequest {
public:
    CGIRequest();
    ~CGIRequest();
    void printEnv();  // debug
    std::string runCgi();

private:
    const std::string _cgiFileName;
    const std::string _cgiPath;
    std::array<std::string, CGI_ENV_SIZE> _CGIEnvArr;
    char *const *_CgiEnvCStyle;

    std::array<std::string, CGI_ENV_SIZE> createCgiEnv();
    char *const *createCgiEnvCStyle();

    // disabled: copy constructor and copy assignment operator
    CGIRequest(const CGIRequest &obj);
    CGIRequest &operator=(const CGIRequest &obj);
};

#endif
