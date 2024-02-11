/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Exception.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/18 14:04:44 by faru          #+#    #+#                 */
/*   Updated: 2024/02/11 03:09:31 by fra           ########   odam.nl         */
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

// NB: for Choko stuff
class ParserException : public WebServerException
{
	public:
		ParserException( std::initializer_list<std::string> const& ) noexcept;
};

class ServerException : public WebServerException
{
	public:
		ServerException( std::initializer_list<std::string> const& ) noexcept;
};

class ExecException : public WebServerException
{
	public:
		ExecException( std::initializer_list<std::string> const& ) noexcept;
};

class HTTPexception : public WebServerException
{
	public:
		HTTPexception( std::initializer_list<std::string> const& ) noexcept;
};

class RequestException : public HTTPexception
{
	public:
		RequestException( std::initializer_list<std::string> const& ) noexcept;
};

class ResponseException : public HTTPexception
{
	public:
		ResponseException( std::initializer_list<std::string> const& ) noexcept;
};