// c++ -Wall -Werror -Wextra CGImain.cpp CGIRequest.cpp

#include "CGIRequest.hpp"
#include <iostream>
#include <string>

int main()
{
    // dummy inputs from config or HTTP request
    std::string CGIfileName = "cgi.sh";
    std::string CGIfilePath = "./cgi.sh";
    std::string serverName = "myServer";

    CGIRequest request(CGIfileName, CGIfilePath, serverName);

    std::string response = request.getHTTPResponse();
    std::cout << response << std::endl;

    return (0);
}
