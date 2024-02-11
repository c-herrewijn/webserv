/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Executor.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:10:43 by fra           #+#    #+#                 */
/*   Updated: 2024/02/11 03:25:55 by fra           ########   odam.nl         */
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

		int				storeRemainingBody( HTTPrequest&, int ) const ;
		HTTPresponse	execRequest( HTTPrequest& ) const;
		HTTPresponse	createResponse( int, std::string) const;

		// void				setRequest( HTTPrequest const& );
		void				setHandler( Server const& );
		// void				setStatus( int )
		// HTTPrequest const&	getRequest( void ) const ;
		Server const&		getHandler( void ) const ;
		// int const&			getStatus( void ) const ;

	private:
		// std::string	_execGET(HTTPrequest&, int&);
		// std::string	_execPOST(HTTPrequest&, int&);
		// std::string	_execDELETE(HTTPrequest&, int&);
		// std::string	_readContent(std::string const&);


		// bool	_isCGI(std::string const&);
		// int		_checkPath(std::string const&, int);

		// const std::map<HTTPmethod, std::function<std::string(void)> > _methods;

		Server		_handler;
		std::string	_servName;
		ssize_t		_maxLenBody;
};
