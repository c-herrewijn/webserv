#include "Exceptions.hpp"

ParserException::ParserException( std::initializer_list<std::string> const& prompts) noexcept
	: WebservException()
{
	this->_msg = "config file parsing error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

ServerException::ServerException( std::initializer_list<std::string> const& prompts) noexcept
	: WebservException()
{
	this->_msg = "server error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
}

HTTPexception::HTTPexception( std::initializer_list<std::string> const& prompts, int status) noexcept
	: WebservException()
{
	this->_msg = "HTTP generic error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
	this->_msg += " - error code: " + std::to_string(status);
	this->_status = status;
}

RequestException::RequestException( std::initializer_list<std::string> const& prompts, int status) noexcept
	: HTTPexception(prompts, status)
{
	this->_msg = "HTTP request error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
	this->_msg += " - error code: " + std::to_string(status);
}

ResponseException::ResponseException( std::initializer_list<std::string> const& prompts, int status) noexcept
	: HTTPexception(prompts, status)
{
	this->_msg = "HTTP response error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
	this->_msg += " - error code: " + std::to_string(status);
}

CGIexception::CGIexception( std::initializer_list<std::string> const& prompts, int status) noexcept
	: HTTPexception(prompts, status)
{
	this->_msg = "HTTP cgi error -";
	for (std::string prompt : prompts)
		this->_msg += " " + prompt;
	this->_msg += " - error code: " + std::to_string(status);
}
