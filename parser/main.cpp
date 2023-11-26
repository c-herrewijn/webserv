/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:13:43 by itopchu       #+#    #+#                 */
/*   Updated: 2023/11/26 14:13:43 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

int main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cout << "1 argument accepted" << std::endl;
		return (1);
	}
	try
	{
		Config config(av[1]);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return (0);
}