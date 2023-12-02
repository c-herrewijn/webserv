/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 05:34:59 by itopchu       #+#    #+#                 */
/*   Updated: 2023/11/26 05:34:59 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP
# include <map>
# include <string>
# include <vector>
# include <unordered_map>
# include <unordered_set>
# include <bitset>
# include "colors.hpp"
# include <iostream>
# ifndef DEF_HOST
#  define DEF_HOST "127.0.0.1"
# endif
# ifndef DEF_PORT
#  define DEF_PORT 80
# endif
# ifndef DEF_SIZE_TYPE
#  define DEF_SIZE_TYPE "M"
# endif
# ifndef DEF_SIZE
#  define DEF_SIZE 32
# endif
# define M_SIZE 3 // amount of methodes used in our program
# define M_GET 1
# define M_POST 2
# define M_DELETE 3
// listen param
struct Listen
{
	int			i_ip;	// Bitwise stored ip
	int			i_port;	// Bitwise stored port
	std::string	s_ip;	// String ip
	std::string	s_port;	// String port
	bool		def;	// Check for default_server
	bool		asteriks;	// If TRUE every IP from 0 to 255 is the ip
};

// Possible duplicate members in server|location
struct Parameters
{
	std::string					max_size;	// Will be overwriten by last found
	bool						autoindex;	// FALSE in default, will be overwriten.
	std::unordered_set<std::string>	index;		// Will be searched in given order
	std::string					root;		// Last found will be used.
	std::unordered_map<size_t, std::string>	error_pages;	// Same status codes will be overwriten
	std::unordered_map<size_t, std::string>	returns;	// Same reponse codes are overwriten by the last
	// cgi_extension
	// // upload
};

//	Location parameters:
struct Location
{
    std::bitset<M_SIZE> allowedMethods;	// Allowed methods
	std::string	URL; // Default "location" param else last given alias
	Parameters	params; // Local values to use. If default it will be a copy of "server" context
	/*
		Non sures:
		cgi_extension
		cgi_pass
		cgi_processing
		fastcgi_param
		fastcgi_pass
		// php_cgi_path
		// python_cgi_path
		cgi_environment_variable
	*/
	std::vector<Location>	nested; // Possible nested locations
};

class Server
{
	private:
		// Parsers
		void	parseListen(std::vector<std::string>& block);
		void	parseServerName(std::vector<std::string>& block);
		void	parseLocation(std::vector<std::string>& block);

		void	parseRoot(std::vector<std::string>& block);
		void	parseBodySize(std::vector<std::string>& block);
		void	parseAutoindex(std::vector<std::string>& block);
		void	parseIndex(std::vector<std::string>& block);
		void	parseErrorPage(std::vector<std::string>& block);
		void	parseReturn(std::vector<std::string>& block);

		void	parseAllowedMethod(std::vector<std::string>& block);
		void	parseAlias(std::vector<std::string>& block);

		bool	clearEmpty(std::vector<std::string>& block);
		void	fillServer(std::vector<std::string>& block);
		// Function pointer type for parsers
		typedef void (Server::*ParserFunction)(std::vector<std::string>&);
		// Map to associate keywords with parser functions
		std::map<std::string, ParserFunction> serverMap;
		std::map<std::string, ParserFunction> paramMap;
		std::map<std::string, ParserFunction> locatMap;
	public:
		// Form
		Server(const Server& copy);
		Server&	operator=(const Server& assign);
		Server(void);
		~Server(void);
		// Subject
		void	parseBlock(std::vector<std::string>& block);
		std::vector<Listen> listens;
		std::vector<std::pair<bool, std::string>> names; // bool is "if (asterix)", std::string is the given "server_name".
		Parameters	params; // Default parameters for whole server block
		std::vector<Location>	location; // declared Locations
		class ErrorCatch : public std::exception {
			public:
				ErrorCatch(const std::string& message) : errorMessage(message) {}
				const char* what() const throw() override {
					return errorMessage.c_str();
				}
			private:
				std::string errorMessage;
		};
};

#endif