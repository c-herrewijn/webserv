/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 17:56:34 by fra           #+#    #+#                 */
/*   Updated: 2023/11/26 03:33:44 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int runWebServer( void )
{
	try
	{
		Server webServ("4242");
		webServ.bindPort();
		webServ.start();
		return (0);
	}
	catch(ServerException const& e)
	{
		std::cout << "Webserver - " << e.what() << "\n";
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