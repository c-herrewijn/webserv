# include <vector>
# include <string>
# include <poll.h>

# define DEF_CONF "../../default/default.conf"

// placeholders
class ServerConfig {};

enum fdType
{
    SERVER_SOCKET,
    CLIENT_CONNECTION,
    CGI_DATA_PIPE,
    CGI_RESPONSE_PIPE,
    STATIC_FILE
};

// classes
class ServerSocket {
    public:
        ServerSocket();
        ServerSocket(ServerConfig &srvConfig);
        void addConnections();

    private:
        struct pollfd _fd;
};

class PollItem
{
    public:
        fdType          _pollType;
        struct pollfd   _fd;
};

struct pollfd *fds createPollFdArr(std::vector<PollItem> pollItems) {
    ;
};


int main()
{
    // data
    std::vector<ServerConfig> serverConfigs = parseServers(DEF_CONF);
    std::vector<ServerSocket> serverSockets = startServers(serverConfigs);
    std::vector<PollItem> pollItems = addServerPollItems(serverSockets);

    while (true)
    {
        struct pollfd *pollFds = createPollFdArr(pollItems);
        poll(pollFds, pollItems.size(), 0);

        for (int i=0; i < pollItems.size(); i++)
        {
            PollItem pItem = findPollItem(pollFds[i]);

            if (pItem._pollType == SERVER_SOCKET)
                addConnections();

            if (pItem._pollType == CLIENT_CONNECTION)
                handleConnection();

            if (pItem._pollType == CGI_DATA_PIPE)
                read_write();


            ...

        }
    }
}
