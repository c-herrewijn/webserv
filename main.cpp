/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 17:56:34 by fra           #+#    #+#                 */
/*   Updated: 2023/11/26 02:36:37 by fra           ########   odam.nl         */
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
		webServ.interactWithClient();
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