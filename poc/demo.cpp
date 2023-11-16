#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

int main() {
    // default settings to create web socket
    int domain = AF_INET;
    int type = SOCK_STREAM;
    int protocol = 0;

    // creating socket for webserver
    std::cout << "creating socket for webserver..." << std::endl;
    int serverFd = socket(domain, type, protocol);

    // define specific ip and port
    struct sockaddr_in intServerSockAddr;
    intServerSockAddr.sin_family = AF_INET;
    intServerSockAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    intServerSockAddr.sin_port = htons(8080);

    // bind the socket to the ip and port
    bind(serverFd, (sockaddr*)&intServerSockAddr, sizeof(intServerSockAddr));

    // todo: Listening and queueing etc..



    // closing socket for webserver
    close(serverFd);
}
