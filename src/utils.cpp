/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/29 16:09:37 by faru          #+#    #+#                 */
/*   Updated: 2024/02/29 16:09:54 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

void	ft_bzero(void *reset, size_t count)
{
	char	*chr_ptr = (char *) reset;

	if (reset == nullptr)
		return ;
	while (count--)
		*chr_ptr++ = '\0';
}