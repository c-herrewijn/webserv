/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   mainServ.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 17:56:34 by fra           #+#    #+#                 */
/*   Updated: 2023/12/06 18:54:10 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include<cmath>
int runWebServer( void )
{
	try
	{
		Server webServ("4242");
		webServ.bindPort();
		webServ.handleMultipleConn();
		return (0);
	}
	catch(ServerException const& e)
	{
		std::cout << e.what() << "\n";
		return (-1);
	}
}

int main( int argc, char** argv, char** envp)
{
	(void) argc;
	(void) argv;
	(void) envp;
	runWebServer();
	return (0);
}