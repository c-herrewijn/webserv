/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:40:36 by fra           #+#    #+#                 */
/*   Updated: 2023/11/30 01:34:04 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "headers.hpp"

class HTTPparser
{
	typedef struct HTTPlist_f
	{
		std::string			key;
		std::string			content;
		struct HTTPlist_f	*next;
	} HTTPlist_t;

	public:
		HTTPparser( void ) noexcept {};
		~HTTPparser( void ) noexcept;

		void	parse( int connfd );
		void	parseHeader( char* );
		void	parseBody( char *, int );
		void	printData( void ) const noexcept;
		std::string const&	getRequest( void ) const noexcept;
		std::string const&	getBody( void ) const noexcept;
		std::string 		getHeader( void ) const noexcept;

	private:
		std::string		_body, _httpReq;
		HTTPlist_t*		_optionalHead = nullptr;

		void	_addNode( char*, char * ) noexcept;
		void	_freeNodes( void ) noexcept;

		HTTPparser( HTTPparser const& ) noexcept;
		HTTPparser& operator=( HTTPparser const& ) noexcept;
};

#include "Server.hpp"