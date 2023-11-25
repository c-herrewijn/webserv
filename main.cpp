/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 17:56:34 by fra           #+#    #+#                 */
/*   Updated: 2023/11/25 18:33:02 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int main( int argc, char** argv, char** envp)
{
	(void) argc;
	(void) argv;
	(void) envp;
	createSocket();
	return (0);
}