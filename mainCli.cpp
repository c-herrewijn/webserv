/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   mainCli.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 20:50:27 by fra           #+#    #+#                 */
/*   Updated: 2024/01/16 12:10:44 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#define STD_REQUEST "GET http://test:21/home/faru/Documents/Codam/webserv/test.txt HTTP/1.1\r\nkey1: value1\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n"
#define STD_FULL "GET http://test:21/halo/find/me/here?bcd=123&gasd=255#sectione HTTP/1.1\r\nkey1: value1\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n"
#define STD_NOHEADS "GET http://test:21/halo/find/me/here HTTP/1.1\r\n\r\nmuch body very http\r\n\r\n"
#define STD_QUERY "GET http://test:21/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nkey1: value1\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n"
void runClient( const char *host, const char *port )
{
	try
	{
		std::cout << "running client on process: " << getpid() << "\n" << STD_REQUEST << '\n'; 
		Client webClient;
		webClient.connectTo(host, port);
		webClient.sendRequest(STD_REQUEST);
	}
	catch(ClientException const& e)
	{
		std::cout << e.what() << "\n";
	}
}

// void runClients(const char *host, const char *port)
// {
// 	pid_t		terminated_pid;
// 	pid_t		curr_pid;
// 	std::set<pid_t> children;
//
// 	int	i = MAX_CLIENTS;
// 	while (i--)
// 	{
// 		curr_pid = fork();
// 		if (curr_pid == -1)
// 		{
// 			std::cerr << "error while forking\n"; 
// 			for (auto childProc : children)
// 				kill(childProc, SIGINT);
// 			return ;
// 		}
// 		else if (curr_pid == 0)
// 		{
// 			runClient(host, port);
// 			sleep(1);
// 			exit(0);
// 		}
// 		else
// 			children.insert(curr_pid);
// 	}
// 	i = MAX_CLIENTS;
// 	while (i--)
// 	{
// 		terminated_pid = wait(NULL);
// 		if (terminated_pid <= 0)
// 		{
// 			std::cerr << "error while terminating process: " << strerror(errno) << "\n"; 
// 			for (auto childProc : children)
// 				kill(childProc, SIGINT);
// 		}
// 		children.erase(terminated_pid);
// 	}
// }

int main( int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "wrong parameters: host and port needed\n";
		return(EXIT_FAILURE);
	}
	runClient(argv[1], argv[2]);
	return(EXIT_SUCCESS);
}