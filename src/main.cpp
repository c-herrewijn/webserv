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

int main(int ac, char **av)
{
	if (ac > 2)
		std::cerr << "Wrong amount of arguments received\n\tValid usage: " << av[0] << " " << av[1] <<  "\n";
	std::vector<Server> servers = parseServers(av);
	WebServer 			webserv(servers);

	webserv.startListen();
	webserv.loop();
	return (0);
}
