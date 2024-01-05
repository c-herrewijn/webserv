#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <vector>
#include <array>

#include "HTTPparser.hpp"
#include "Server.hpp"

#define BUFFER_SIZE 10000 // hardcoded for poc
#define CGI_ENV_SIZE 18

std::vector<Server>	parseServers(char **av);

std::array<std::string, CGI_ENV_SIZE> createCgiEnv(std::string serverName)
{
    std::array<std::string, CGI_ENV_SIZE> CGIEnv {
        "AUTH_TYPE=",
        "CONTENT_LENGTH=",
        "CONTENT_TYPE=",
        "GATEWAY_INTERFACE=1.1", //
        "PATH_INFO=",
        "PATH_TRANSLATED=",
        "QUERY_STRING=", //
        "REMOTE_ADDR=", //
        "REMOTE_HOST=",
        "REMOTE_IDENT=",
        "REMOTE_USER=",
        "REQUEST_METHOD=", //
        "SCRIPT_NAME=", //
        "SERVER_NAME=" + serverName, //
        "SERVER_PORT=", //
        "SERVER_PROTOCOL=", //
        "SERVER_SOFTWARE=", //
        "HTTP_COOKIE=", //
    };
    return CGIEnv;
}

char **createCgiEnvCStyle(std::array<std::string, CGI_ENV_SIZE> CGIEnvArr)
{
    int i = 0;
    char **CgiEnv = new char*[CGI_ENV_SIZE + 1];
    while (i < CGI_ENV_SIZE)
    {
        CgiEnv[i] = new char[CGIEnvArr[i].length()+1];
        strncpy(CgiEnv[i], CGIEnvArr[i].c_str(), CGIEnvArr[i].length()+1);
        i++;
    }
    CgiEnv[i] = NULL;
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

    std::array<std::string, CGI_ENV_SIZE> cgiEnvArr = createCgiEnv("MyServer");
    char **CgiEnvCStyle = createCgiEnvCStyle(cgiEnvArr);

	pid_t childPid = fork();
	if (childPid == 0)
	{
	    close(p1[0]);
        dup2(p1[1], STDOUT_FILENO);
        char *argv[2] = {(char*)cgiFileName.c_str(), NULL};
        int res = execve(cgiPath.c_str(), argv, CgiEnvCStyle);
        if (res != 0)
        {
            close(p1[1]);
            std::cerr << "execve error!" << std::endl;
            exit(1); // exit() is not allowed!
        }
	}
    delete[] CgiEnvCStyle;

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
