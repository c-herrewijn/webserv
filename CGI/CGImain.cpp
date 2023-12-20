// c++ -Wall -Werror -Wextra CGImain.cpp CGIRequest.cpp

#include "CGIRequest.hpp"
#include <iostream>
#include <string>

int main()
{
    CGIRequest request;
    std::string response = request.runCgi();
    std::cout << response << std::endl;

    return (0);
}
