/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Exception.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/18 14:04:58 by faru          #+#    #+#                 */
/*   Updated: 2024/02/09 00:06:00 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Exception.hpp"

ServerException::ServerException( std::initializer_list<std::string> const& prompts) noexcept
{
	this->_msg = "server error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

RequestException::RequestException( std::initializer_list<std::string> const& prompts) noexcept
{
	this->_msg = "request parsing error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

ExecException::ExecException( std::initializer_list<std::string> const& prompts) noexcept
{
	this->_msg = "request execution error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

ResponseException::ResponseException( std::initializer_list<std::string> const& prompts) noexcept
{
	this->_msg = "response building error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}
