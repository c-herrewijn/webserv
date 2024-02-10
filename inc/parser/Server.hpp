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
# include "HTTPrequest.hpp"
# include "HTTPresponse.hpp"

# define DEF_CONF "default/default.conf"

class Server
{
	public:
		Server(const Server& copy);
		Server&	operator=(const Server& assign);
		Server(void) {};
		virtual ~Server(void);

		void			parseBlock(std::vector<std::string>& block);
		void			executeRequest(HTTPrequest&) const;
		HTTPresponse	sendResponse(int, std::string&) const ;
	
		const std::vector<Listen>&		getListens(void) const;
		const std::vector<std::string>& getNames(void) const;
		const Parameters&				getParams(void) const;
		const std::vector<Location>&	getLocations(void) const;
		const std::string& 				getCgiDir(void) const;
		const std::string& 				getCgiExtension(void) const;
		const bool& 					getCgiAllowed(void) const;

		class ErrorCatch : public std::exception {
			public:
				ErrorCatch(const std::string& message) : errorMessage(message) {}
				const char* what() const throw() override {
					return errorMessage.c_str();
				}
			private:
				std::string errorMessage;
		};

	private:
		std::vector<Listen> 		listens; // Listens
		std::vector<std::string>	names; // is the given "server_name".
		Parameters					params; // Default parameters for whole server block
		std::vector<Location>		locations; // declared Locations
		std::string					cgi_directory;	// bin for cgi
		std::string					cgi_extension;	// extention .py .sh
		bool						cgi_allowed;	// Check for permissions

		void	_parseListen(std::vector<std::string>& block);
		void	_parseServerName(std::vector<std::string>& block);
		void	_parseLocation(std::vector<std::string>& block);
		void	_parseCgiDir(std::vector<std::string>& block);
		void	_parseCgiExtension(std::vector<std::string>& block);
		void	_parseCgiAllowed(std::vector<std::string>& block);

		void	_fillServer(std::vector<std::string>& block);

    friend std::ostream& operator<<(std::ostream& os, const Server& server);
};

#endif