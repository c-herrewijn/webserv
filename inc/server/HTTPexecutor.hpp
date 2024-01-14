/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPexecutor.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:10:43 by fra           #+#    #+#                 */
/*   Updated: 2024/01/14 18:57:08 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "HTTPparser.hpp"
#include <iostream>
#include <unistd.h>
#include <functional>
#include <filesystem>          	// filesystem::path, access()
#include <sys/types.h>        	// chdir
#include <sys/stat.h>         	// stat
#include <dirent.h>           	// opendir, readdir, closedir
#include <fcntl.h>				// for access constants
#include <fstream>
#define CGI_DIR					std::filesystem::path("var/www/cgi-bin")
#define CGI_EXT_DEFAULT			std::filesystem::path(".cgi")
#define CGI_EXT_PY				std::filesystem::path(".py")
#define CGI_EXT_SH				std::filesystem::path(".sh")

class ExecException : public HTTPexception
{
	public:
		ExecException( std::initializer_list<const char*>) noexcept;
};

class HTTPexecutor
{
	public:
		static void	execRequest(HTTPrequest&);
		~HTTPexecutor( void ) noexcept {};

	private:
		static	void		_execGET(HTTPrequest&);
		static	void		_execPOST(HTTPrequest&);
		static	void		_execDELETE(HTTPrequest&);
		static	std::string	_readContent(std::filesystem::path&);

		static	bool	_isCGI(std::filesystem::path const&);
		static	void	_checkPath(std::filesystem::path const&);

		HTTPexecutor( void ) noexcept {};
		HTTPexecutor( HTTPexecutor const& ) noexcept;
		HTTPexecutor& operator=( HTTPexecutor const& ) noexcept;

		static const std::map<HTTPmethod, std::function<void(HTTPrequest&)> > _methods;
};

