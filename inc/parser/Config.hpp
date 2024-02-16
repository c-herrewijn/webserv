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
# include "Exceptions.hpp"

class Config
{
	public:
		Config(void);
		virtual	~Config();
		Config(const Config& copy);
		Config&	operator=(const Config& assign);

		void									fillConfig(const std::string& file);
		bool									clearEmpty(void);
		std::vector<std::string>				getFileContent(void);
		std::vector<std::vector<std::string>>	divideContent(void);

	private:
		size_t	_doComment(size_t &i);
		size_t	_doSpace(size_t& i);
		void	_doQuote(size_t& i, size_t& j);
		void	_doToken(size_t& i, size_t& j);
		void	_doExceptions(size_t& i);
		void	_doClean(void);
		void	_readFile(const std::string& file_path);
		void	_tokenizeFile(void);
		void	_checkBrackets(void);
		void	_printContent(void);

		std::vector<std::string>	file_content;
		std::string 				raw_input;
};

#endif