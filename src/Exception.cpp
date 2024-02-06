/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Exception.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/18 14:04:58 by faru          #+#    #+#                 */
/*   Updated: 2024/02/06 09:26:02 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Exception.hpp"

ServerException::ServerException( std::initializer_list<std::string> const& prompts) noexcept
{
	this->_msg = "server error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

ParserException::ParserException( std::initializer_list<std::string> const& prompts) noexcept
{
	this->_msg = "parsing error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

ExecException::ExecException( std::initializer_list<std::string> const& prompts) noexcept
{
	this->_msg = "request execution error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

BuilderException::BuilderException( std::initializer_list<std::string> const& prompts) noexcept
{
	this->_msg = "response builder error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}
