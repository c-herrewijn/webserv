/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPexecutor.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:11:07 by fra           #+#    #+#                 */
/*   Updated: 2023/12/31 16:48:47 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPexecutor.hpp"

ExecException::ExecException( std::initializer_list<const char*> prompts) noexcept 
	: HTTPexception(prompts)
{
	this->_msg = "parsing error -";
	for (const char *prompt : prompts)
		this->_msg += " " + std::string(prompt);
}

HTTPexecutor::HTTPexecutor( HTTPexecutor const& other ) noexcept
{
	(void) other;
}

HTTPexecutor& HTTPexecutor::operator=( HTTPexecutor const& other ) noexcept
{
	(void) other;
	return (*this);
}
