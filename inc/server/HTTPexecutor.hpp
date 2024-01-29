/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPexecutor.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:10:43 by fra           #+#    #+#                 */
/*   Updated: 2024/01/29 10:59:46 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <unistd.h>
#include <functional>
#include <sys/types.h>        	// chdir
#include <sys/stat.h>         	// stat
#include <dirent.h>           	// opendir, readdir, closedir
#include <fcntl.h>				// for access constants
#include <fstream>
#include "Exception.hpp"
#include "HTTPstructs.hpp"
#include "define.hpp"

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

		static	bool	_isCGI(std::string const&);
		static	int		_checkPath(std::string const&, int);

		HTTPexecutor( void ) noexcept {};
		HTTPexecutor( HTTPexecutor const& ) noexcept;
		HTTPexecutor& operator=( HTTPexecutor const& ) noexcept;

		static const std::map<HTTPmethod, std::function<std::string(HTTPrequest&, int&)> > _methods;
};

