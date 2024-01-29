/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Exception.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/18 14:04:44 by faru          #+#    #+#                 */
/*   Updated: 2024/01/29 18:13:39 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <initializer_list>

typedef enum statusRequest_t
{
	REQ_OK,
	REQ_CGI,
	REQ_ERROR_PARSE,
	REQ_ERROR_EXEC,
	REQ_ERROR_SOCK,
	REQ_ERROR_RESP,
	REQ_ERROR_GEN,
} statusRequest;

class WebServerException : public std::exception
{
	public:
		WebServerException( void ) noexcept : std::exception() {};
		virtual const char* what() const noexcept override {return (this->_msg.c_str());}
		virtual ~WebServerException( void ) noexcept {};
		statusRequest const& getType( void ) const {return (this->_type);};

	protected:
		std::string 	_msg;
		statusRequest	_type;
};

class ServerException : public WebServerException
{
	public:
		ServerException( std::initializer_list<const char*> ) noexcept;
};

class ParserException : public WebServerException
{
	public:
		ParserException( std::initializer_list<const char*> ) noexcept;
};

class ExecException : public WebServerException
{
	public:
		ExecException( std::initializer_list<const char*>) noexcept;
};

class BuilderException : public WebServerException
{
	public:
		BuilderException( std::initializer_list<const char*>) noexcept;
};