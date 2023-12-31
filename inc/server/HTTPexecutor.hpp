/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPexecutor.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/31 11:10:43 by fra           #+#    #+#                 */
/*   Updated: 2023/12/31 15:08:48 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "HTTPparser.hpp"

class ExecException : public HTTPexception
{
	public:
		ExecException( std::initializer_list<const char*>) noexcept;
};

class HTTPexecutor
{
	public:
		static void	execRequest(HTTPrequest&);
		~HTTPexecutor( void ) noexcept {};

	private:
		HTTPexecutor( void ) noexcept {};
		HTTPexecutor( HTTPexecutor const& ) noexcept;
		HTTPexecutor& operator=( HTTPexecutor const& ) noexcept;
};

