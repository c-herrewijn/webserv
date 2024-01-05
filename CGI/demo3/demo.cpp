#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <vector>

#include "HTTPparser.hpp"
#include "Server.hpp"

#define BUFFER_SIZE 10000 // hardcoded for poc

std::vector<Server>	parseServers(char **av);

char *const *createCgiEnv()
{
    char *const *CgiEnv = new char*[19] {
        (char *)"AUTH_TYPE=",
        (char *)"CONTENT_LENGTH=",
        (char *)"CONTENT_TYPE=",
        (char *)"GATEWAY_INTERFACE=1.1", //
        (char *)"PATH_INFO=",
        (char *)"PATH_TRANSLATED=",
        (char *)"QUERY_STRING=",//
        (char *)"REMOTE_ADDR=", //
        (char *)"REMOTE_HOST=",
        (char *)"REMOTE_IDENT=",
        (char *)"REMOTE_USER=",
        (char *)"REQUEST_METHOD=", //
        (char *)"SCRIPT_NAME=", //
        (char *)"SERVER_NAME=MyServer", //
        (char *)"SERVER_PORT=", //
        (char *)"SERVER_PROTOCOL=", //
        (char *)"SERVER_SOFTWARE=WebServ", //
        (char *)"HTTP_COOKIE=", //
        NULL
    };
    return CgiEnv;
}

// info needed:
// - info for ENV ??
// - info to find and call the cgi script (name and path)
std::string runCgi(HTTPrequest &req, Server &srv)
{
    const std::string cgiFileName = "cgi.py";
    const std::string cgiPath = "./cgi.py";

    std::cout << "cgiDir:" << srv.getCgiDir() << std::endl;
    std::cout << "CgiExtension:" << srv.getCgiExtension() << std::endl;
    int p1[2];
	char read_buff[BUFFER_SIZE];
    bzero(read_buff, BUFFER_SIZE); // bzero() is not allowed!

    // run cgi, and write result into pipe
	pipe(p1);
    char *const *CgiEnv = createCgiEnv();
	pid_t childPid = fork();
	if (childPid == 0)
	{
	    close(p1[0]);
        dup2(p1[1], STDOUT_FILENO);
        char *argv[2] = {(char*)cgiFileName.c_str(), NULL};
        // std::cerr << "debug: " << cgiPath << std::endl;
        int res = execve(cgiPath.c_str(), argv, CgiEnv);
        if (res != 0)
        {
            close(p1[1]);
            std::cerr << "execve error!" << std::endl;
            exit(1); // exit() is not allowed!
        }
	}
    delete[] CgiEnv;

    // return cgi response
    int	stat_loc;
    close(p1[1]);
    waitpid(childPid, &stat_loc, 0);
    read(p1[0], read_buff, BUFFER_SIZE);
    close(p1[0]);
    std::string response = read_buff;
    return response;
}

int main(int argc, char *argv[]) {
    // default settings to create web socket
    int domain = AF_INET;
    int type = SOCK_STREAM;
    int protocol = 0;

    std::vector<Server> servers = parseServers(argv);
    Server myServer = servers[0]; // note: config only used for CGI in this demo

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
    std::cout << "wait for incoming connections..." << std::endl;
    HTTPrequest	req;
    std::string reqStr;
    while (true)
    {
        connectionFd = accept(serverFd, (sockaddr*)&intServerSockAddr, &socketSize);
        std::cout << "connection established!" << std::endl;

        bytesReceived = read(connectionFd, buffer, BUFFER_SIZE);
        // std::cout << std::endl << "data received: " << std::endl<< buffer << std::endl;

        reqStr = buffer;
        HTTPparser::parseRequest(reqStr, req);

        // HTTPparser::printData(req);
        std::cout << "parsing finished" << std::endl;


        // run CGI to determine te response string
        std::string responseStr = runCgi(req, myServer);

        // write response:
        // - check via browser: http://localhost:8080/
        // - check via curl: $ curl -d "myRequestFromCurl" localhost:8080
        if(write(connectionFd, (void *)responseStr.c_str(), strlen(responseStr.c_str())) < 0)
        {
            std::cerr << "Error: " << strerror(errno) << std::endl;
            return 1;
        }
        close(connectionFd); // closing connection
        break; // dummy,server stops after first connection has been established, normally, it should keep running
    }

    // closing socket for webserver
    close(serverFd);
}
