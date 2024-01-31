#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <array>
#include <sstream>

#include "HTTPparser.hpp"
#include "Server.hpp"
#include "CGI.hpp"

#define BUFFER_SIZE 10000 // hardcoded for poc

std::vector<Server>	parseServers(char **av);

std::string create_response(HTTPrequest	&req, Server &serverCfg)
{
    std::cout << "requested path: " << req.head.url.path << std::endl;
    // HTTPparser::printData(*req);

    // Only create CGI object if after the following validations (TODO):
    // - check if the uri is a cgi uri (based on nested "Location" info in server config)
    // - check if CGI is allowed (in server config, based on file extention and/or CGI allowed flag)
    // - check if CGIfile exists and is executable

    // create response
    std::istringstream ss((req).head.url.path);
    std::string reqExtension;
    while (getline(ss, reqExtension, '.')) {} // get last part after '.'
    std::string responseStr;
    if (reqExtension == serverCfg.getCgiExtension()
        || "." + reqExtension == serverCfg.getCgiExtension())
    {
        // CGI
        std::cout << "doing cgi..." << std::endl;
        CGI CGIrequest(req, serverCfg);
        responseStr = CGIrequest.getHTTPResponse();
    }
    else {
        if (req.head.url.path != "/favicon.ico") { // browser always asks for favicon.ico; skipping it for now;
            // Static page
            HTTPresponse	response;
            std::string     htmlBody;
            std::cout << "doing static page..." << std::endl;
            int reqStat = HTTPexecutor::execRequest(req, htmlBody);
            response = HTTPbuilder::buildResponse(reqStat, htmlBody);
            responseStr = response.toString();
        }
    }
    return responseStr;
}

int main(int argc, char *argv[]) {
    // default settings to create web socket
    int domain = AF_INET;
    int type = SOCK_STREAM;
    int protocol = 0;

    std::vector<Server> servers = parseServers(argv);
    Server myServerConfig = servers[0]; // note: config only used for CGI in this demo

    // creating socket for webserver (returns a file descriptor)
    std::cout << "launching webserver..." << std::endl;
    int serverFd = socket(domain, type, protocol);

    // set additional option: SO_REUSEADDR to prevent time-out
    int optvalTrue = true;
    if(setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &optvalTrue, sizeof(optvalTrue)) < 0)
    {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        return 1;
    }

    // define specific ip and port
    struct sockaddr_in intServerSockAddr;
    intServerSockAddr.sin_family = AF_INET;
    intServerSockAddr.sin_addr.s_addr = INADDR_ANY;
    intServerSockAddr.sin_port = htons(8080);

    // bind the socket to the ip and port
    socklen_t socketSize = sizeof(intServerSockAddr);
    std::cout << "binding... " << std::endl;
    if(bind(serverFd, (sockaddr*)&intServerSockAddr, socketSize) < 0)
    {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        return 1;
    }

    // listen to incoming connections
    int maxNrOfConnections = 42;
    std::cout << "listening... " << std::endl;
    if (listen(serverFd, maxNrOfConnections) < 0)
    {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        return 1;
    }

    // server can now be reached via browser on addr: http://0.0.0.0:8080
    int connectionFd;
    char buffer[BUFFER_SIZE] = {0};
    int bytesReceived;

    // accept incoming connections (server waits for connections in a loop)
    // - check via curl: $ curl -d "myRequestFromCurl" localhost:8080
    std::cout << "wait for incoming connections..." << std::endl;
    std::cout << "test via: http://localhost:8080" << std::endl;
    std::string reqStr;
    while (true)
    {
        bzero(buffer, BUFFER_SIZE);
        connectionFd = accept(serverFd, (sockaddr*)&intServerSockAddr, &socketSize);
        std::cout << "connection established!" << std::endl;
        bytesReceived = read(connectionFd, buffer, BUFFER_SIZE);
        reqStr = buffer;
        HTTPrequest	*req = new HTTPrequest;
		try
		{
            HTTPparser::parseRequest(reqStr, *req);
            std::string responseStr = create_response(*req, myServerConfig);
            // write response:
            if (write(connectionFd, (void *)responseStr.c_str(), strlen(responseStr.c_str())) < 0)
                std::cerr << "Error: " << strerror(errno) << std::endl;
            close(connectionFd); // closing connection
            delete req;

		}
		catch(const std::exception& e)
		{
			std::cerr << C_RED << "Error in handling request: " << C_RESET "\n";
			std::cerr << C_YELLOW << e.what() << C_RESET "\n";
			std::cerr << C_YELLOW "Continuing with parsing other requests...\n" C_RESET;
            close(connectionFd); // closing connection
            delete req;
		}
    }
    // closing socket for webserver
    close(serverFd);
}
