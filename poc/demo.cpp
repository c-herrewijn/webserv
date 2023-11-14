#include <sys/socket.h>
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


    // closing socket for webserver
    close(serverFd);

}
