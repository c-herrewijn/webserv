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
# include "Server.hpp"
class Config
{
	private:
		// Form
		Config(void);
		Config(const Config& copy);
		Config&	operator=(const Config& assign);
		// Subject
		size_t	doComment(size_t &i);
		size_t	doSpace(size_t& i);
		void	doQuote(size_t& i, size_t& j);
		void	doToken(size_t& i, size_t& j);
		void	doExceptions(size_t& i);
		void	doClean(void);
		void	tokenizeFile(void);
		void	parseContent(void);
		void	readFile(char* file_path);
		std::vector<std::string>	file_content;
		std::vector<Server>	pool;
		std::string raw_input;
	public:
		virtual	~Config();
		Config(char* file);
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