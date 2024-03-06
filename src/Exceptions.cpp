/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Exceptions.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/18 14:04:58 by faru          #+#    #+#                 */
/*   Updated: 2024/03/06 10:08:19 by fra           ########   odam.nl         */
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

HTTPexception::HTTPexception( std::initializer_list<std::string> const& prompts, int status) noexcept
	: WebServerException()
{
	this->_msg = "HTTP parsing error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
	this->_status = status;
}

int	HTTPexception::getStatus( void ) const
{
	return (this->_status);
}

RequestException::RequestException( std::initializer_list<std::string> const& prompts, int status) noexcept
	: HTTPexception(prompts, status)
{
	this->_msg = "request error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

ResponseException::ResponseException( std::initializer_list<std::string> const& prompts, int status) noexcept
	: HTTPexception(prompts, status)
{
	this->_msg = "response error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

CGIexception::CGIexception( std::initializer_list<std::string> const& prompts, int status) noexcept
	: HTTPexception(prompts, status)
{
	this->_msg = "cgi error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}
