/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Config.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 21:15:44 by itopchu       #+#    #+#                 */
/*   Updated: 2023/11/25 21:15:44 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Server.hpp"

Config::Config(void) {}

Config::~Config(void)
{
	pool.clear();
	file_content.clear();
}

Config::Config(std::string &file)
{
	tokenizeFile(readFile(file));
	parseContent();
}

Config& Config::operator=(const Config& assign)
{
	pool.clear();
	file_content.clear();
	pool = assign.pool;
	file_content = assign.file_content;
	return (*this);
}

Config::Config(const Config& copy) :
	file_content(copy.file_content),
	pool(copy.pool)
{

}

std::vector<Server>&	Config::getPool(void)
{
	return (pool);
}

std::vector<std::string>&	Config::getFileContent(void)
{
	return (file_content);
}

std::string&	Config::readFile(std::string& file_path)
{
	std::ifstream inputFile(file_path);
	if (!inputFile.is_open())
		throw ErrorCatch(("Error opening the file: " + file_path).c_str());

	std::ostringstream	fileContentStream;
	fileContentStream << inputFile.rdbuf();
	if (inputFile.bad())
		throw ErrorCatch(("Error reading the file: " + file_path).c_str());

	std::string	fileContent = fileContentStream.str();
	inputFile.close();
	if (fileContent.empty())
		throw ErrorCatch(("The file is empty: " + file_path).c_str());
	return (fileContent);
}

size_t	Config::doComment(std::string& raw_input, size_t &i)
{
	if (raw_input[i] != '#')
		return (i);
	while (raw_input[i] != '\n' && i < raw_input.size())
		i++;
	return (i);
}

void	Config::doQuote(std::string& raw_input, size_t& i, size_t& j)
{
	if (raw_input[i] != '\'' && raw_input[i] != '"')
		return ;
	j++;
	while (j < raw_input.size() && raw_input[j] != raw_input[i])
		j++;
	if (j >= raw_input.size())
		throw ErrorCatch("Non-matching quote.");
	file_content.emplace_back(raw_input.cbegin() + i, raw_input.cbegin() + j);
	i = j;
}

size_t	Config::doSpaces(std::string& raw_input, size_t& i)
{
	if (!std::isspace(raw_input[i]))
		return (i);
	while (i < raw_input.size() && std::isspace(raw_input[i]))
		i++;
	if (file_content.size() == 0)
		return (i);
	file_content.push_back(" ");
	return (i);
}

void	Config::doExceptions(std::string& raw_input, size_t& i)
{
	if ((raw_input[i] >= 1 && 8 <= raw_input[i]) || (raw_input[i] >= 14 && raw_input[i] <= 31))
		throw ErrorCatch("Invalid character in the file.");
}

void	Config::doToken(std::string& raw_input, size_t& i, size_t& j)
{
	if (j >= raw_input.size()
		|| std::isspace(raw_input[j])
		|| raw_input[j] == '"'
		|| raw_input[j] == '\''
		|| raw_input[j] == '#')
		return ;
	while (j < raw_input.size()
		&& !std::isspace(raw_input[j])
		&& raw_input[j] != '"'
		&& raw_input[j] != '\''
		&& raw_input[j] != '#'
		&& raw_input[j] != ';')
		j++;
	if (j == i)
		return ;
	if (j - 1 >= i)
		file_content.emplace_back(raw_input.cbegin() + i, raw_input.cbegin() + j - 1);
	if (raw_input[j] == ';')
		file_content.emplace_back(";");
	i = j;	
}

void	Config::tokenizeFile(std::string& raw_input)
{
	size_t	i = 0, j = 0;
	while (i < raw_input.size())
	{
		doExceptions(raw_input, i);
		j = doSpaces(raw_input, i);
		j = doComment(raw_input, i);
		doQuote(raw_input, i, j);
		doToken(raw_input, i, j);
	}
	if (!file_content.size())
		throw ErrorCatch("No valuable input found in given config file.");
}

void	Config::parseContent(void)
{
	size_t i = 0, j = 0;
	while (i < file_content.size())
	{
		i++;
	}
}