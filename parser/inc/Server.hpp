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
# include <unordered_map>
# include <unordered_set>
# include <iostream>

# include "colors.hpp"
# include "Parameters.hpp"
# include "Location.hpp"
# include "Listen.hpp"

# define DEF_CONF "default/default.conf"

class Server
{
	private:
		// Parsers
		void	parseListen(std::vector<std::string>& block);
		void	parseServerName(std::vector<std::string>& block);
		void	parseLocation(std::vector<std::string>& block);
		void	parseParams(std::vector<std::string>& block);
		// Setup
		bool	clearEmpty(std::vector<std::string>& block);
		void	fillServer(std::vector<std::string>& block);
	public:
		// Form
		Server(const Server& copy);
		Server&	operator=(const Server& assign);
		Server(void);
		virtual ~Server(void);
		// Subject
		void	parseBlock(std::vector<std::string>& block);
		std::vector<Listen> listens;
		std::vector<std::string> names; // is the given "server_name".
		Parameters	params; // Default parameters for whole server block
		std::vector<Location>	location; // declared Locations
		/*
			To Do:
			getListens
			getNames
			getParams
			getLocation
		*/

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