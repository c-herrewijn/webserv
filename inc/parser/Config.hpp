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
# include <cctype>
# include <stack>
# include "Server.hpp"

class Config
{
	private:
		size_t	doComment(size_t &i);
		size_t	doSpace(size_t& i);
		void	doQuote(size_t& i, size_t& j);
		void	doToken(size_t& i, size_t& j);
		void	doExceptions(size_t& i);
		void	doClean(void);
		void	readFile(const std::string& file_path);
		void	tokenizeFile(void);
		void	checkBrackets(void);
		void	printContent(void);
		std::vector<std::string>	file_content;
		std::string 				raw_input;
	public:
		// Form
		Config(void);
		virtual	~Config();
		Config(const Config& copy);
		Config&	operator=(const Config& assign);
		// Subject
		void	fillConfig(const std::string& file);
		bool	clearEmpty(void);
		std::vector<std::string>	getFileContent(void);
		std::vector<std::vector<std::string>>	divideContent(void);
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