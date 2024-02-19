/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Exception.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/18 14:04:58 by faru          #+#    #+#                 */
/*   Updated: 2024/01/22 23:39:44 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Exception.hpp"

ServerException::ServerException( std::initializer_list<const char*> prompts) noexcept
{
	this->_msg = "server error -";
	for (const char *prompt : prompts)
		this->_msg += " " + std::string(prompt);
	this->_type = REQ_ERROR_GEN;
}

ParserException::ParserException( std::initializer_list<const char*> prompts) noexcept
{
	this->_msg = "parsing error -";
	for (const char *prompt : prompts)
		this->_msg += " " + std::string(prompt);
	this->_type = REQ_ERROR_PARSE;
}

ExecException::ExecException( std::initializer_list<const char*> prompts) noexcept
{
	this->_msg = "request execution error -";
	for (const char *prompt : prompts)
		this->_msg += " " + std::string(prompt);
	this->_type = REQ_ERROR_EXEC;
}

BuilderException::BuilderException( std::initializer_list<const char*> prompts) noexcept
{
	this->_msg = "response builder error -";
	for (const char *prompt : prompts)
		this->_msg += " " + std::string(prompt);
	this->_type = REQ_ERROR_RESP;
}
