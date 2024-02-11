/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Exceptions.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/18 14:04:58 by faru          #+#    #+#                 */
/*   Updated: 2024/02/11 03:16:16 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Exceptions.hpp"

ParserException::ParserException( std::initializer_list<std::string> const& prompts) noexcept
	: WebServerException()
{
	this->_msg = "config file parsing error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

ServerException::ServerException( std::initializer_list<std::string> const& prompts) noexcept
	: WebServerException()
{
	this->_msg = "server error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

ExecException::ExecException( std::initializer_list<std::string> const& prompts) noexcept
	: WebServerException()
{
	this->_msg = "request execution error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

HTTPexception::HTTPexception( std::initializer_list<std::string> const& prompts) noexcept
	: WebServerException()
{
	this->_msg = "HTTP parsing error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

RequestException::RequestException( std::initializer_list<std::string> const& prompts) noexcept
	: HTTPexception(prompts)
{
	this->_msg = "request error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

ResponseException::ResponseException( std::initializer_list<std::string> const& prompts) noexcept
	: HTTPexception(prompts)
{
	this->_msg = "response building error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}
