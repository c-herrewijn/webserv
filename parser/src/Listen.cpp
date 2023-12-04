/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Listen.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/03 01:47:47 by itopchu       #+#    #+#                 */
/*   Updated: 2023/12/03 01:47:47 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Listen.hpp"
#include <iostream>

Listen::Listen(const Listen& copy) :
	i_ip(copy.i_ip),
	i_port(copy.i_port),
	s_ip(copy.s_ip),
	s_port(copy.s_port),
	def(copy.def)
{

}

Listen&	Listen::operator=(const Listen& assign)
{
	i_ip = assign.i_ip;
	i_port = assign.i_port;
	s_ip = assign.s_ip;
	s_port = assign.s_port;
	def = assign.def;
	return (*this);
}

Listen::Listen(void)
{
	i_ip = {0, 0, 0, 0};
	i_port = 0;
	s_ip = "0.0.0.0";
	s_port = "0";
	def = false;
	all = false;
}

Listen::~Listen(void)
{
	i_ip.clear();
}

void	Listen::fillFull(std::vector<std::string>& block)
{
	uint8_t counter = 0;
	uint16_t	tmp = 0;
	if (!std::isdigit(block.front().front()))
		ErrorCatch("First element is not a digit in listen '" + block.front() + "'");
	for (size_t i = 0; block.front()[i]; i++)
	{
		while (std::isdigit(block.front()[i]))
		{
			tmp = tmp * 10 + block.front()[i] - '0';
			if (tmp > 255)
				throw ErrorCatch("Ip range is too high on '" + block.front() + "'");
			i++;
		}
		i_ip.push_back(tmp);
		tmp = 0;
		if (block.front()[i] == '.')
			counter++;
		else if (block.front()[i] == ':')
		{
			i++;
			while (std::isdigit(block.front()[i]))
			{
				tmp = tmp * 10 + block.front()[i] - '0';
				if (tmp > MAX_PORT)
					throw ErrorCatch("Port is too high on '" + block.front() + "'");
				i++;
			}
			if (block.front()[i])
				throw ErrorCatch("Unexpected or missing element in '" + block.front() + "'");
			i_port = tmp;
			s_port = std::to_string(tmp);
		}
		if (counter > 3)
			throw ErrorCatch("'listen' has more '.' than expected on '" + block.front() + "'");
	}
	if (counter != 3)
		throw ErrorCatch("'listen' has less '.' than expected on '" + block.front() + "'");
	block.erase(block.begin());
	s_ip = std::to_string(i_ip[0]) + "." + std::to_string(i_ip[1]) + "." + std::to_string(i_ip[2]) + "." + std::to_string(i_ip[3]);
}

void	Listen::fillIp(std::vector<std::string>& block)
{
	s_port = DEF_PORT;
	i_port = std::stoi(DEF_PORT);
	uint8_t counter = 0;
	uint16_t	tmp = 0;
	if (!std::isdigit(block.front().front()))
		ErrorCatch("First element is not a digit in listen '" + block.front() + "'");
	for (size_t i = 0; block.front()[i]; i++)
	{
		while (std::isdigit(block.front()[i]))
		{
			tmp = tmp * 10 + block.front()[i] - '0';
			if (tmp > 255)
				throw ErrorCatch("Ip range is too high on '" + block.front() + "'");
			i++;
		}
		i_ip.push_back(tmp);
		tmp = 0;
		if (block.front()[i] == '.')
			counter++;
		if (counter > 3)
			throw ErrorCatch("'listen' has more '.' than expected on '" + block.front() + "'");
	}
	if (counter != 3)
		throw ErrorCatch("'listen' has less '.' than expected on '" + block.front() + "'");
	block.erase(block.begin());
	s_ip = std::to_string(i_ip[0]) + "." + std::to_string(i_ip[1]) + "." + std::to_string(i_ip[2]) + "." + std::to_string(i_ip[3]);
}

void	Listen::fillPort(std::vector<std::string>& block)
{
	all = true;
	if (block.front().find_first_of(":") != block.front().find_last_of(":"))
		ErrorCatch("Expected 'port' type '*:80' or ':80' or '8000' on '" + block.front() + "'");
	if (block.front().front() == '*')
		block.front().erase(block.front().begin());
	if (block.front().front() == ':')
		block.front().erase(block.front().begin());
	for (size_t i = 0; i < block.front().size(); i++)
	{
		if (!std::isdigit(block.front()[i]))
			ErrorCatch("Unexpected character '" + std::to_string(block.front()[i]) + "' on '" + block.front() + "'");
		i_port = i_port * 10 + block.front()[i] - '0';
		if (i_port > 65535)
			ErrorCatch("Port is too big on '" + block.front() + "'");
	}
	block.erase(block.begin());
}

void	Listen::fillValues(std::vector<std::string>& block)
{
	if (block.front().find_first_not_of("*:0123456789") != std::string::npos)
		ErrorCatch("Unexpected character in listen '" + block.front() + "'");
	bool hasDot = false;
	bool hasColumn = false;
	if (block.front().find_first_of(".") != std::string::npos)
		hasDot = true;
	if (block.front().find_first_of(":") != std::string::npos)
		hasColumn = true;
	if (hasDot && hasColumn)
		fillFull(block);
	else if (hasDot)
		fillIp(block);
	else
		fillPort(block);
}

const std::string&	Listen::getIpString(void)
{
	return (s_ip);
}

const std::vector<uint8_t>&	Listen::getIpInt(void)
{
	return (i_ip);
}

const std::string&	Listen::getPortString(void)
{
	return (s_port);
}

const uint16_t&	Listen::getPortInt(void)
{
	return (i_port);
}

const bool&	Listen::getAll(void)
{
	return (all);
}

const bool&	Listen::getDef(void)
{
	return (def);
}

void	Listen::setDef(bool	status)
{
	def = status;
}

void	Listen::setAll(bool	status)
{
	all = status;
}
