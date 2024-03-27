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

# define DEF_CONF std::string("default/def.conf")

std::vector<ConfigServer>	parseServers(std::string const& fileName)
{
	Config *config;
	config = new Config();
	std::vector<ConfigServer> servers;
	try {
		config->fillConfig(fileName);
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
		delete config;
		return (servers);
	}
	std::vector<std::vector<std::string>> separated = config->divideContent();
	delete config;
	for (size_t i = 0; i < separated.size(); i++)
	{
		ConfigServer tmp;
		std::cout << "--Parsing Server index " C_GREEN << i << C_RESET "--\n";
		try {
			tmp.parseBlock(separated[i]);
			servers.push_back(tmp);
		}
		catch(const std::exception& e) {
			std::cerr << "Failure on Server index " C_RED << i << C_RESET "\n";
			std::cerr << C_RED << e.what() << C_RESET "\n";
			std::cerr << C_YELLOW "Continuing with parsing other servers...\n" C_RESET;
		}
	}
	return (servers);
}

int main(int ac, char **av)
{
	std::vector<ConfigServer> servers;
	std::cout << av[0] << '\n';
	if (ac == 1)
	{
		std::cout << "No argument provided, using default configuration in " C_GREEN << DEF_CONF << C_RESET "\n";
		servers = parseServers(DEF_CONF);
	}
	else if (ac == 2)
	{
		std::cout << "Using custom configuration in " C_GREEN << av[1] << C_RESET "\n";
		servers = parseServers(av[1]);
		if (servers.empty())
		{
			std::cout << C_RED "No valid server configuration in " << av[1] << C_RESET "\nSwitching to default configuration in " C_GREEN << DEF_CONF << C_RESET "\n";
			servers = parseServers(DEF_CONF);
		}
	}
	else
	{
		std::cerr << C_RED "Wrong amount of arguments - valid usage: ./" << av[0] << " [config_file_path]\n";
		return (EXIT_FAILURE);
	}
	std::cout << "Found " C_AZURE << servers.size() << C_RESET " available server\n";

	try
	{
		WebServer	webserv(servers);
		webserv.run();
	}
	catch(const WebservException& e) {
		std::cerr << e.what() << '\n';
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
