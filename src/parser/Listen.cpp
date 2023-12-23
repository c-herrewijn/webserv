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
	i_port(copy.i_port),
	i_ip(copy.i_ip),
	s_ip(copy.s_ip),
	s_port(copy.s_port),
	def(copy.def),
	all(copy.all)
{

}

Listen&	Listen::operator=(const Listen& assign)
{
	i_ip = assign.i_ip;
	i_port = assign.i_port;
	s_ip = assign.s_ip;
	s_port = assign.s_port;
	def = assign.def;
	all = assign.all;
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
	all = 0;
	if (!std::isdigit(block.front().front()))
		throw ErrorCatch("First element is not a digit in listen '" + block.front() + "'");
	uint8_t counter = 0;
	uint16_t	tmp = 0;
	int i = 0;
	while (block.front()[i] != '\0' && std::isdigit(block.front()[i]))
	{
		while (std::isdigit(block.front()[i]))
		{
			tmp = tmp * 10 + block.front()[i] - '0';
			if (tmp > 255)
				throw ErrorCatch("Ip range is too high on '" + block.front() + "'");
			i++;
		}
		i_ip[counter] = tmp;
		tmp = 0;
		if (block.front()[i] == '.')
		{
			i++;
			counter++;
		}
		if (counter > 3)
			throw ErrorCatch("'listen' has more '.' than expected on '" + block.front() + "'");
	}
	if (counter != 3)
		throw ErrorCatch("'listen' has less '.' than expected on '" + block.front() + "'");
	if (block.front()[i++] != ':')
		throw ErrorCatch("Unexpected character on: '" + block.front() + "'");
	if (!std::isdigit(block.front()[i]))
		throw ErrorCatch("Unexpected or missing character on: '" + block.front() + "'");
	uint32_t port = 0;
	while (std::isdigit(block.front()[i]))
	{
		port = port * 10 + block.front()[i] - '0';
		if (port > 65535)
			throw ErrorCatch("Port is too big on '" + block.front() + "'");
		i++;
	}
	if (block.front()[i] != '\0')
		throw ErrorCatch("Unexpected character on '" + block.front() + "'");
	i_port = port;
	s_port = std::to_string(port);
	block.erase(block.begin());
	s_ip = std::to_string(i_ip[0]) + "." + std::to_string(i_ip[1]) + "." + std::to_string(i_ip[2]) + "." + std::to_string(i_ip[3]);
}

void	Listen::fillIp(std::vector<std::string>& block)
{
	if (!std::isdigit(block.front().front()))
		throw ErrorCatch("First element is not a digit in listen '" + block.front() + "'");
	s_port = DEF_PORT;
	i_port = std::stoi(DEF_PORT);
	uint8_t counter = 0;
	uint16_t	tmp = 0;
	int i = 0;
	while (block.front()[i] != '\0' && std::isdigit(block.front()[i]))
	{
		while (std::isdigit(block.front()[i]))
		{
			tmp = tmp * 10 + block.front()[i] - '0';
			if (tmp > 255)
				throw ErrorCatch("Ip range is too high on '" + block.front() + "'");
			i++;
		}
		i_ip[counter] = tmp;
		tmp = 0;
		if (block.front()[i] == '.')
		{
			i++;
			counter++;
		}
		if (counter > 3)
			throw ErrorCatch("'listen' has more '.' than expected on '" + block.front() + "'");
	}
	if (block.front()[i] != '\0')
		throw ErrorCatch("Unexpected character on: '" + block.front() + "'");
	if (counter != 3)
		throw ErrorCatch("'listen' has less '.' than expected on '" + block.front() + "'");
	block.erase(block.begin());
	s_ip = std::to_string(i_ip[0]) + "." + std::to_string(i_ip[1]) + "." + std::to_string(i_ip[2]) + "." + std::to_string(i_ip[3]);
}

void	Listen::fillPort(std::vector<std::string>& block)
{
	all = true;
	if (block.front().find_first_of(":") != block.front().find_last_of(":"))
		throw ErrorCatch("Expected 'port' type '*:80' or ':80' or '8000' on '" + block.front() + "'");
	if (block.front().front() == '*')
		block.front().erase(block.front().begin());
	if (block.front().front() == ':')
		block.front().erase(block.front().begin());
	uint32_t port = 0;
	int i = 0;
	while (std::isdigit(block.front()[i]))
	{
		port = port * 10 + block.front()[i] - '0';
		if (port > 65535)
			throw ErrorCatch("Port is too big on '" + block.front() + "'");
		i++;
	}
	if (block.front()[i] != '\0')
		throw ErrorCatch("Unexpected character on '" + block.front() + "'");
	i_port = port;
	s_port = std::to_string(port);
	block.erase(block.begin());
}

void	Listen::fillValues(std::vector<std::string>& block)
{
	if (block.front().find_first_not_of("*:.0123456789") != std::string::npos)
		throw ErrorCatch("Unexpected character in listen '" + block.front() + "'");
	if (!std::isdigit(block.front()[0]) && block.front()[0] != '*')
		throw ErrorCatch("After 'listen' a digit or '*' expected. Fault on '" + block.front() + "'");
	bool hasDot = false;
	bool hasColumn = false;
	if (block.front().find('.') != std::string::npos)
		hasDot = true;
	if (block.front().find(':') != std::string::npos)
		hasColumn = true;
	if (block.front().find('*') != std::string::npos && (hasDot || !hasColumn))
		throw ErrorCatch("Faulty parameter in 'listen': '" + block.front() + "'");
	if (hasDot && hasColumn)
		fillFull(block);
	else if (hasDot)
		fillIp(block);
	else
		fillPort(block);
}

const std::string&	Listen::getIpString(void) const
{
	return (s_ip);
}

const std::vector<uint8_t>&	Listen::getIpInt(void) const
{
	return (i_ip);
}

const std::string&	Listen::getPortString(void) const
{
	return (s_port);
}

const uint16_t&	Listen::getPortInt(void) const
{
	return (i_port);
}

const bool&	Listen::getAll(void) const
{
	return (all);
}

const bool&	Listen::getDef(void) const
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

std::ostream& operator<<(std::ostream& os, const Listen& listen) {
    os << "listen " << listen.getIpString();
    if (!listen.getPortString().empty()) {
        os << ":" << listen.getPortString();
    }
    if (listen.getDef()) {
        os << " default_server";
    }
    os << ";\n";

    return os;
}
