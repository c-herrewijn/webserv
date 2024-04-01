#include "parser/Tokenizer.hpp"
#include "server/WebServer.hpp"

std::vector<Config>	parseServers(std::string const& fileName)
{
	Tokenizer *config;
	config = new Tokenizer();
	std::vector<Config> servers;
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
		Config tmp;
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

static void assignDefaults(std::vector<Config>& servers)
{
	std::vector<Listen const*> candidates;
	for (auto& server : servers)
	{
		for (auto const& listen : server.getListens())
		{
			bool	contains = false;
			for (auto it = candidates.begin(); it != candidates.end(); ++it)
			{
				if (listen.getIpString() == (*it)->getIpString() &&
					listen.getPortString() == (*it)->getPortString())
				{
					contains = true;
					if (!(*it)->getDef() && listen.getDef())
					{
						candidates.push_back(&listen);
						it = candidates.erase(it);
					}
				}
			}
			if (!contains)
				candidates.push_back(&listen);
		}
	}
	for (auto const& candidate : candidates)
		const_cast<Listen*>(candidate)->setDef(true);
}

int main(int ac, char **av)
{
	std::vector<Config> servers;
	std::string	file = DEF_CONF;
	if (ac != 1 && ac !=2)
	{
		std::cerr << C_RED "Wrong amount of arguments - valid usage: ./" << av[0] << " [config_file_path]\n";
		return (EXIT_FAILURE);
	}
	if (ac == 2)
		file = av[1];
	std::cout << "Using config: " << C_GREEN << file << C_RESET << "\n";
	servers = parseServers(file);
	// Can be commented
	std::cout << "Default listens:\n";
	for (auto const& server : servers)
	{
		std::cout << "\t";
		for (auto const& name : server.getNames())
			std::cout << name << " ";
		std::cout << "\n";
		for (auto const& listen : server.getListens())
			std::cout << "\t\t" << listen.getIpString() << ":" << listen.getPortString() << "\n";
	}
	if (servers.empty() && file != DEF_CONF)
	{
		std::cout << C_RED "No valid server configuration in " << file << C_RESET "\nSwitching to default configuration in " C_GREEN << DEF_CONF << C_RESET "\n";
		servers = parseServers(DEF_CONF);
	}
	assignDefaults(servers);
	std::cout << "Found " C_AZURE << servers.size() << C_RESET " valid server(s)\n";

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
