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
#include "ConfigServer.hpp"

Config::Config(void) {}

Config::~Config(void) {}

void	Config::fillConfig(const std::string& file)
{
	_readFile(file);
	_tokenizeFile();
	if (clearEmpty())
		throw ParserException({"config file is empty"});
	_checkBrackets();
}

std::vector<std::vector<std::string>>	Config::divideContent(void)
{
	std::vector<std::vector<std::string>>	ret;
	std::vector<std::string> tmp;
	int	bracks = 0;
	for (std::vector<std::string>::iterator it = file_content.begin(); it != file_content.end(); it++)
	{
		tmp.push_back(*it);
		if (*it == "{")
			bracks++;
		else if (*it == "}")
		{
			bracks--;
			if (bracks == 0)
			{
				ret.push_back(std::move(tmp));
				tmp.clear();
			}
		}
	}
	return (ret);
}

Config& Config::operator=(const Config& assign)
{
	raw_input = assign.raw_input;
	file_content.clear();
	file_content = assign.file_content;
	return (*this);
}

Config::Config(const Config& copy) :
	file_content(copy.file_content),
	raw_input(copy.raw_input)
{

}

std::vector<std::string>	Config::getFileContent(void)
{
	return (file_content);
}

void	Config::_readFile(const std::string& file_path)
{
	std::ifstream inputFile(file_path);
	if (!inputFile.is_open())
		throw ParserException({"error opening file:", file_path});

	std::ostringstream	fileContentStream;
	fileContentStream << inputFile.rdbuf();
	if (inputFile.bad())
		throw ParserException({"error reading file:", file_path});

	raw_input = fileContentStream.str();
	inputFile.close();
	if (raw_input.empty())
		throw ParserException({"empty file", file_path,});
}

size_t	Config::_doComment(size_t &i)
{
	if (raw_input[i] != '#')
		return (i);
	while (raw_input[i] != '\n' && i < raw_input.size())
		i++;
	return (i);
}

void	Config::_doQuote(size_t& i, size_t& j)
{
	if (raw_input[i] != '\'' && raw_input[i] != '"')
		return ;
	char	type = raw_input[i];
	j = i + 1;
	while (raw_input[j] && raw_input[j] != type)
		j++;
	if (raw_input[j] == type)
	{
		j++;
		file_content.push_back(raw_input.substr(i, j - i));
	}
	else
		throw ParserException({"non-matching quote"});
	i = j;
}

size_t	Config::_doSpace(size_t& i)
{
	if (!std::isspace(raw_input[i]))
		return (i);
	while (i < raw_input.size() && std::isspace(raw_input[i]))
		i++;
	if (file_content.size() != 0)
		file_content.push_back(" ");
	return (i);
}

void	Config::_doExceptions(size_t& i)
{
	if ((raw_input[i] >= 1 && 8 <= raw_input[i]) || (raw_input[i] >= 14 && raw_input[i] <= 31))
		throw ParserException({"invalid character in file"});
}

void	Config::_doToken(size_t& i, size_t& j)
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
	if (j - 1 >= i)
		file_content.emplace_back(raw_input.cbegin() + i, raw_input.cbegin() + j);
	if (raw_input[j] == ';')
	{
		j++;
		file_content.emplace_back(";");
	}
	i = j;
}

void	Config::_doClean()
{
	for (std::vector<std::string>::iterator it = file_content.begin(); it != file_content.end();)
	{
		if (*it == " ")
			it = file_content.erase(it);
		else
			it++;
	}
	size_t pos = 0;
	while (pos < file_content.size())
	{
		if (file_content[pos].front() == '\'' || file_content[pos].front() == '\"')
			file_content[pos].erase(file_content[pos].begin());
		if (!file_content[pos].empty()
			&& (file_content[pos].back() == '\'' || file_content[pos].back() == '\"'))
			file_content[pos].pop_back();
		if (file_content[pos].empty())
			file_content.erase(file_content.begin() + pos);
		else
			pos++;
	}
}

void	Config::_tokenizeFile(void)
{
	size_t	i = 0, j = 0;
	while (i < raw_input.size())
	{
		j = _doSpace(i);
		j = _doComment(i);
		j = _doSpace(i);
		_doQuote(i, j);
		j = _doSpace(i);
		_doToken(i, j);
	}
	if (file_content.size() == 0)
		throw ParserException({"no valuable input found in given config file"});
	_doClean();
}

void	Config::_checkBrackets(void)
{
	int bracks = 0;
	std::vector<std::string>::iterator it;
	for (it = file_content.begin(); it != file_content.end(); ++it)
	{
		if (*it == "{")
			bracks++;
		else if (*it == "}")
		{
			if (--bracks < 0)
				throw ParserException({"mismatched brackets"});
		}
	}
	if (bracks)
		throw ParserException({"missing brackets"});
}

void	Config::_printContent(void)
{
	size_t i = 0;
	while (i < file_content.size())
	{
		std::cout << file_content[i];
		i++;
	}
	std::cout << std::endl;
}

bool	Config::clearEmpty(void)
{
	for (std::vector<std::string>::iterator it = file_content.begin(); it != file_content.end();)
	{
		if (*it == " ")
			it = file_content.erase(it);
		else
			++it;
	}
	return file_content.empty();
}