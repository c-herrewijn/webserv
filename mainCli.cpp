/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   mainCli.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 20:50:27 by fra           #+#    #+#                 */
/*   Updated: 2023/11/26 21:15:34 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

void runWebClient( const char *port, const char *host )
{
	try
	{
		Client webClient(port, host);
		webClient.findServer();
		webClient.sendRequest("GET / HTTP/1.1\r\nkey1: value1\r\nkey2: value2\r\n\r\nmuch body very http\n");
	}
	catch(ClientException const& e)
	{
		std::cout << "Webclient - " << e.what() << "\n";
	}
}

int main( int argc, char** argv)
{
	if (argc != 3)
		std::cout << "wrong parameters: port and host needed\n";
	else
		runWebClient(argv[1], argv[2]);
	return (0);
}