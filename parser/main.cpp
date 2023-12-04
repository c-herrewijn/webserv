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
	for (std::vector<std::vector<std::string>>::iterator it = separated.begin(); it != separated.end(); it++)
	{
		Server tmp;
		try
		{
			tmp.parseBlock(*it);
			servers.push_back(tmp);
		}
		catch(const std::exception& e)
		{
			std::cerr << "Failure on Server index: " << std::distance(separated.begin(), it) << "\n";
			std::cerr << C_RED << e.what() << C_RESET "\n";
			std::cerr << "Continuing with parsing other servers...\n";
		}
	}
	// "servers" must contain valid servers
	return (0);
}

// c++ main.cpp src/*.cpp -Linc -Iinc -I../inc -L../inc -o runner
// ./runner default/default.conf