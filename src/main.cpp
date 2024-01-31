/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:13:43 by itopchu       #+#    #+#                 */
/*   Updated: 2023/11/26 14:13:43 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "parser/Config.hpp"
#include "server/WebServer.hpp"

std::vector<Server>	parseServers(char **av)
{
	Config *config;
	config = new Config();
	std::vector<Server> servers;
	try
	{
		if (av[1])
			config->fillConfig(av[1]);
		else
			config->fillConfig(DEF_CONF);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		delete config;
		return (servers);
	}
	std::vector<std::vector<std::string>> separated = config->divideContent();
	delete config;
	for (size_t i = 0; i < separated.size(); i++)
	{
		Server tmp;
		try
		{
			std::cout << "--Parsing Server index " C_GREEN << i << C_RESET "--\n";
			tmp.parseBlock(separated[i]);
			servers.push_back(tmp);
		}
		catch(const std::exception& e)
		{
			std::cerr << "Failure on Server index " C_RED << i << C_RESET "\n";
			std::cerr << C_RED << e.what() << C_RESET "\n";
			std::cerr << C_YELLOW "Continuing with parsing other servers...\n" C_RESET;
		}

	}
	std::cout << "Parsing Done with size " C_AZURE << servers.size() << C_RESET "\n";
	// "servers" must contain valid servers
	return (servers);
}

int runWebServer( void )
{
	try
	{
		WebServer webServ("HAL-9001");
		webServ.listenTo("localhost","4242");
		webServ.listenTo("localhost","4343");
		webServ.listenTo("localhost","4444");
		webServ.loop();
		return (0);
	}
	catch(ServerException const& e)
	{
		std::cout << e.what() << "\n";
		return (-1);
	}
}

void	testReqs( void )
{
	HTTPrequest request;
	std::vector<const char*>	testReqs({
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://test:21/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://test:21/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:81\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		"GET http://test/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:81\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://test:21/halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nkey1: value1\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\nContent-Type: text/plain\r\nContent-Length: 12much body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
		// "GET http://halo/find/me/here?amd=123&def=566 HTTP/1.1\r\nHost: domin:23\r\nkey2: value2\r\n\r\nmuch body very http\r\n\r\n",
	});
	std::cout << "===========================================================================================\n";
	for (auto req : testReqs)
	{
		try
		{
			request = HTTPparser::parseRequest(req);
			std::cout << req << "---------n\n" << request.toString();
		}
		catch(const ParserException& e)
		{
			std::cerr << e.what() << '\n' << req;
		}
		std::cout << "===========================================================================================" << '\n';
	}
}

int main(int ac, char **av)
{
	if (ac > 2)
	{
		std::cerr << "Wrong amount of arguments received\n\tValid usage: " << av[0] << " " << av[1] <<  "\n";
	}
	std::vector<Server> servers = parseServers(av);
	// for (size_t i = 0; i < servers.size(); i++)
	// {
	// 	std::cout << "---Printing Server index: "  C_GREEN << i << C_RESET "---\n";
	// 	std::cout << servers[i];
	// }
	// runWebServer();
	testReqs();
	return (0);
}
