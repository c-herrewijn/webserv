/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Executor.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:10:43 by fra           #+#    #+#                 */
/*   Updated: 2024/02/16 10:15:27 by faru          ########   odam.nl         */
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
#include <sys/types.h>        // send, recv
#include <sys/socket.h>       // send, recv
#include <strings.h>
#include <initializer_list>

#include "Exceptions.hpp"
#include "HTTPrequest.hpp"
#include "HTTPresponse.hpp"
#include "Server.hpp"

class Executor
{
	public:

		static HTTPresponse	execRequest( HTTPrequest&, Server const& ) noexcept;
		static HTTPresponse	createResponse( int, std::string const&, std::string const&) noexcept;

	private:
		Executor( void ) noexcept {};
		~Executor( void ) noexcept {};

		static std::string	_runMethod(HTTPrequest const&);
		static std::string	_execGET(HTTPrequest const&);
		static std::string	_execPOST(HTTPrequest const&);
		static std::string	_execDELETE(HTTPrequest const&);
		static std::string	_readContent(std::string const&);
};
