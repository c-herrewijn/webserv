/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPexecutor.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:10:43 by fra           #+#    #+#                 */
/*   Updated: 2024/01/16 12:54:33 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "HTTPparser.hpp"
#include <iostream>
#include <unistd.h>
#include <functional>
// #include <filesystem>          	// filesystem::path, access()
#include <sys/types.h>        	// chdir
#include <sys/stat.h>         	// stat
#include <dirent.h>           	// opendir, readdir, closedir
#include <fcntl.h>				// for access constants
#include <fstream>
// #define CGI_DIR					std::filesystem::path("var/www/cgi-bin")
// #define CGI_EXT_DEFAULT			std::filesystem::path(".cgi")
// #define CGI_EXT_PY				std::filesystem::path(".py")
// #define CGI_EXT_SH				std::filesystem::path(".sh")

class ExecException : public HTTPexception
{
	public:
		ExecException( std::initializer_list<const char*>) noexcept;
};

class HTTPexecutor
{
	public:
		static int	execRequest(HTTPrequest&, std::string&);
		~HTTPexecutor( void ) noexcept {};

	private:
		static	std::string	_execGET(HTTPrequest&, int&);
		static	std::string	_execPOST(HTTPrequest&, int&);
		static	std::string	_execDELETE(HTTPrequest&, int&);
		static	std::string	_readContent(std::string const&);

		// static	bool	_isCGI(std::filesystem::path const&);
		static	void	_checkPath(std::string const&);

		HTTPexecutor( void ) noexcept {};
		HTTPexecutor( HTTPexecutor const& ) noexcept;
		HTTPexecutor& operator=( HTTPexecutor const& ) noexcept;

		static const std::map<HTTPmethod, std::function<std::string(HTTPrequest&, int&)> > _methods;
};

