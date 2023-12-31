/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPexecutor.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:10:43 by fra           #+#    #+#                 */
/*   Updated: 2023/12/31 11:16:18 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "HTTPparser.hpp"

class HTTPexecutor
{
	public:
		static void	execRequest(HTTPRequest&);

	private:
		HTTPexecutor(/* args */);
		HTTPexecutor(/* args */);
		~HTTPexecutor();
		HTTPexecutor&	operator=(HTTPexecutor const&);
};

