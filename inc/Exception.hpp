/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Exception.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/18 14:04:44 by faru          #+#    #+#                 */
/*   Updated: 2024/02/06 09:25:31 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <initializer_list>

class WebServerException : public std::exception
{
	public:
		WebServerException( void ) noexcept : std::exception() {};
		virtual const char* what() const noexcept override {return (this->_msg.c_str());}
		virtual ~WebServerException( void ) noexcept {};

	protected:
		std::string 	_msg;
};

class ServerException : public WebServerException
{
	public:
		ServerException( std::initializer_list<std::string> const& ) noexcept;
};

class ParserException : public WebServerException
{
	public:
		ParserException( std::initializer_list<std::string> const& ) noexcept;
};

class ExecException : public WebServerException
{
	public:
		ExecException( std::initializer_list<std::string> const& ) noexcept;
};

class BuilderException : public WebServerException
{
	public:
		BuilderException( std::initializer_list<std::string> const& ) noexcept;
};