/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:40:36 by fra           #+#    #+#                 */
/*   Updated: 2023/11/26 19:55:51 by fra           ########   odam.nl         */
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
		HTTPparser( void ) {};
		~HTTPparser( void );

		void	parse( int connfd );
		void	parseHeader( char* );
		void	parseBody( int );

	private:
		std::string		_body, _httpReq, _httpResp;
		HTTPlist_t*		_header = nullptr;

		void	_addNode( char* );
		void	_freeNodes( void );
		HTTPparser( HTTPparser const& );
		HTTPparser& operator=( HTTPparser const& );
};

#include "Server.hpp"