/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Executor.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:10:43 by fra           #+#    #+#                 */
/*   Updated: 2024/02/20 15:56:19 by faru          ########   odam.nl         */
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
#include <filesystem>

#include "RequestValidate.hpp"
#include "Exceptions.hpp"
#include "HTTPrequest.hpp"
#include "HTTPresponse.hpp"
#include "ConfigServer.hpp"

class Executor
{
	public:
		Executor( ConfigServer const&, HTTPrequest& ) noexcept;
		~Executor( void ) noexcept {};
		HTTPresponse	execRequest( void ) noexcept;
		HTTPresponse	createResponse( int, std::string const&) noexcept;

		ConfigServer const&		getHandler( void ) const noexcept;
		void				setRequest( HTTPrequest& ) noexcept;
		HTTPrequest const&	getRequest( void ) const noexcept;

	private:
		ConfigServer const&		_configServer;
		std::string 		_servName;
		HTTPrequest&		_request;

		std::string	_runHTTPmethod( void );
		std::string	_execGET( void );
		std::string	_execPOST( void );
		std::string	_execDELETE( void );
		std::string	_readContent(std::string const&);
};
