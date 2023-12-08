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

#include "Config.hpp"

int main(int ac, char **av)
{
	Config *config;

	config = new Config();
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
		return (1);
	}
	std::vector<std::vector<std::string>> separated = config->divideContent();
	delete config;
	std::vector<Server> servers;
	for (int i = 0; i < separated.size(); i++)
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
	for (int i = 0; i < servers.size(); i++)
	{
		std::cout << "---Printing Server index: "  C_GREEN << i << C_RESET "---\n";
		std::cout << servers[i] << std::endl;
	}
	return (0);
}

// c++ main.cpp src/*.cpp -Linc -Iinc -I../inc -L../inc -o runner
// ./runner default/default.conf
