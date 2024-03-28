#pragma once
#include <string>
#include <initializer_list>

class WebservException : public std::exception
{
	public:
		WebservException( void ) noexcept : std::exception() {};
		virtual const char* what() const noexcept override {return (this->_msg.c_str());}
		virtual ~WebservException( void ) noexcept {};

	protected:
		std::string 	_msg;
};

class ParserException : public WebservException
{
	public:
		ParserException( std::initializer_list<std::string> const& ) noexcept;
};

class ServerException : public WebservException
{
	public:
		ServerException( std::initializer_list<std::string> const& ) noexcept;
};

class EndConnectionException : public WebservException
{
	public:
		EndConnectionException( void ) noexcept {};
};

class HTTPexception : public WebservException
{
	public:
		HTTPexception( std::initializer_list<std::string> const&, int ) noexcept;
		int	getStatus( void ) const {return (this->_status);}

	protected:
		int	_status;
};

class RequestException : public HTTPexception
{
	public:
		RequestException( std::initializer_list<std::string> const&, int ) noexcept;
};

class ResponseException : public HTTPexception
{
	public:
		ResponseException( std::initializer_list<std::string> const&, int ) noexcept;
};

class CGIexception : public HTTPexception
{
	public:
		CGIexception( std::initializer_list<std::string> const&, int ) noexcept;
};
