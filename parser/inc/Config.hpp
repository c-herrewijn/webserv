/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Config.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 11:44:46 by itopchu       #+#    #+#                 */
/*   Updated: 2023/11/25 11:44:46 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP
# include <iostream>
# include <fstream>
# include <sstream>
# include <string>
# include <exception>
# include <vector>
# include <cctype>
# include <stack>
class Server;
class Config
{
	private:
		// Form
		Config(void);
		virtual	~Config();
		Config(const Config& copy);
		Config&	operator=(const Config& assign);
		// Subject
		size_t	doComment(std::string& raw_input, size_t &i);
		size_t	doSpaces(std::string& raw_input, size_t& i);
		void	doQuote(std::string& raw_input, size_t& i, size_t& j);
		void	doToken(std::string& raw_input, size_t& i, size_t& j);
		void	doExceptions(std::string& raw_input, size_t& i);
		void	tokenizeFile(std::string& raw_input);
		void	parseContent(void);
		std::string&	readFile(std::string& file_path);
		std::vector<std::string>	file_content;
		std::vector<Server>	pool;
	public:
		Config(std::string &file);
		std::vector<Server>&	getPool(void);
		std::vector<std::string>&	getFileContent(void);
		class ErrorCatch : public std::exception {
			public:
				ErrorCatch(const char* message) : errorMessage(message) {}
				const char* what() const throw() override {
					return errorMessage.c_str();
				}
			private:
				std::string errorMessage;
	};
};

#endif