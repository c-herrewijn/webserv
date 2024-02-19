#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <poll.h>
#include <sstream>

#define BUFFER_SIZE 50
#define QUEUE_LENGTH 42
#define TIMEOUT_MS 1000

#define C_RED   "\x1b[31m"
#define C_GREEN "\x1b[32m"
#define C_RESET "\x1b[0m"

static void printPollResult(struct pollfd *pollItems, nfds_t nfds) {
    std::cout << "poll result of monitoring '" << nfds << "' pollItems: " << std::endl;
    for (nfds_t i=0; i < nfds; i++) {
        if (pollItems[i].revents != 0)
        {
            std::cout << " - fd " << pollItems[i].fd << ":" << pollItems[i].revents;
            if (pollItems[i].revents & POLLIN)
                std::cout << " POLLIN (There is data to read);";
            if (pollItems[i].revents & POLLOUT)
                std::cout << " POLLOUT (Writing is now possible);";
            std::cout << std::endl;
        }
    }
}

int main() {
    // default settings to create web socket
    int domain = AF_INET;
    int type = SOCK_STREAM;
    int protocol = 0;
    int serverFd = socket(domain, type, protocol);
    // set additional option: SO_REUSEADDR to prevent time-out
    int optvalTrue = true;
    if(setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &optvalTrue, sizeof(optvalTrue)) < 0)
    {
        return 1;
    }

    // define specific ip and port
    sockaddr_in intServerSockAddr;
    intServerSockAddr.sin_family = AF_INET;
    intServerSockAddr.sin_addr.s_addr = INADDR_ANY;
    intServerSockAddr.sin_port = htons(8080);

    // bind the socket to the ip and port
    socklen_t socketSize = sizeof(intServerSockAddr);
    std::cout << "binding... " << std::endl;
    if(bind(serverFd, (sockaddr*)&intServerSockAddr, socketSize) < 0)
    {
        return 1;
    }
    std::cout << "serverFd " << serverFd << std::endl;

    // listen to incoming connections
    std::cout << "listening... " << std::endl;
    if (listen(serverFd, QUEUE_LENGTH) < 0)
    {
        return 1;
    }

    // server can now be reached via browser on addr: http://0.0.0.0:8080
    char buffer[BUFFER_SIZE] = {0};
    int bytesReceived;
    int bytesSent;

    // accept incoming connections (server waits for connections in a loop)
    // - check via curl: $ curl -d "myRequestFromCurl" localhost:8080
    std::cout << "wait for incoming connections..." << std::endl;
    std::cout << "test via: http://localhost:8080" << std::endl;

    // create a queue;
    pollfd pollItems[QUEUE_LENGTH + 1]; // 1 poll item for every connection + 1 for the server itself, to poll for new connections
    int res;
    pollItems[0].fd = serverFd;
    pollItems[0].events = POLLIN | POLLOUT;
    int nrOfConnections = 0;
    int connectionFd;
    while (true) // loops forever to keep server running
    {
        sleep(1); // not per se needed, easier for testing

        // poll the status of the fds (all connectionFds + serverFd);
        res = poll(pollItems, nrOfConnections + 1, TIMEOUT_MS);
        if (res != 0)
            printPollResult(pollItems, nrOfConnections + 1);
        if (pollItems[0].revents & POLLIN) {

            // add new connection; NOTE: this demo does not include logic to drop the connection
            connectionFd = accept(serverFd, (sockaddr*)&intServerSockAddr, &socketSize);
            nrOfConnections++;
            std::cout << C_GREEN << "connection established, fd: " << connectionFd << C_RESET << std::endl;

            // accepted connections are added to the queue
            // NOTE: normally, connections that are closed are also removed
            pollItems[nrOfConnections].fd = connectionFd;
            pollItems[nrOfConnections].events = POLLIN | POLLOUT;

            // NOTE: normally, server would never stop, and slots would be freed after connection closes.
            if (nrOfConnections == QUEUE_LENGTH)
                break;
        }

        // read / write to connections (no logic yet)
        for (int i=1; i < nrOfConnections + 1; i++) { // i starts at 1, to skip the serverFd
            if (pollItems[i].revents & POLLIN) {
                bzero(buffer, BUFFER_SIZE);
                bytesReceived = read(pollItems[i].fd, buffer, BUFFER_SIZE);
                std::cout << "read " << bytesReceived << " bytes from fd: " << pollItems[i].fd << std::endl;
            }
            if (pollItems[i].revents & POLLOUT) { // POLLOUT means connection is ready to receive response
                bytesSent = write(pollItems[i].fd, "test123\r\n", 9);
                std::cout << "sent " << bytesSent << " bytes to fd: " << pollItems[i].fd << std::endl;
            }
        }
    }
}
