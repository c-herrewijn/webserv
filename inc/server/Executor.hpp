/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Executor.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:10:43 by fra           #+#    #+#                 */
/*   Updated: 2024/02/15 17:59:13 by fra           ########   odam.nl         */
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
		Executor( void ) noexcept;
		Executor( Server const& ) noexcept ;
		~Executor( void ) noexcept {};

		HTTPresponse	execRequest( HTTPrequest& ) const noexcept;
		HTTPresponse	createResponse( int, std::string) const noexcept;

		void				setHandler( Server const& ) noexcept;
		Server const&		getHandler( void ) const noexcept;

	private:
		std::string	_execGET(HTTPrequest&) const;
		std::string	_execPOST(HTTPrequest&) const;
		std::string	_execDELETE(HTTPrequest&) const;
		std::string	_readContent(std::string const&) const;

		Server		_handler;
		std::string	_servName;
		ssize_t		_maxLenBody;
};
