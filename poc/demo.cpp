#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

#define BUFFER_SIZE 10000 // hardcoded for poc

int main() {
    // default settings to create web socket
    int domain = AF_INET;
    int type = SOCK_STREAM;
    int protocol = 0;

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
    // accept incoming connections (server waits for connections in a loop)
    std::cout << "wait for incoming connections..." << std::endl;
    int connectionFd;
    char buffer[BUFFER_SIZE] = {0};
    int bytesReceived;
    while (true)
    {
        connectionFd = accept(serverFd, (sockaddr*)&intServerSockAddr, &socketSize);
        std::cout << "connection established!" << std::endl;

        bytesReceived = read(connectionFd, buffer, BUFFER_SIZE);
        std::cout << std::endl << "data received: " << std::endl<< buffer << std::endl;

        // todo:
        // - create + send response

        close(connectionFd); // closing connection
        break; // dummy,server stops after first connection has been established, normally, it should keep running
    }

    // closing socket for webserver
    close(serverFd);
}
