/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPparser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 14:40:36 by fra           #+#    #+#                 */
/*   Updated: 2023/12/01 02:57:20 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "headers.hpp"

typedef enum HTTPreqStatus_s
{
    FMT_OK,
    FMT_BADFMT,			//"empty header"
    FMT_BIGHEAD,		//"header exceeds 8 KB maximum"
    FMT_BADOPT,			//"bad format option header line"
	FMT_EOF,			//"closed connection"
} HTTPreqStatus_t;

typedef struct HTTPrequest_f
{
	std::string			request;
	std::list<std::pair<std::string, std::string> > options;
	std::string			body;
	bool				hasOpts = false;
	bool				hasBody = false;
} HTTPrequest_t;

class HTTPparser
{
	public:
		static HTTPreqStatus_t	parse( int connfd, HTTPrequest_t& ) ;
		static void				printData( HTTPrequest_t ) noexcept;
		static const char*		printStatus( HTTPreqStatus_t ) noexcept;

	private:
		static HTTPreqStatus_t	_getOptions( char*, std::list<std::pair<std::string, std::string> >& ) noexcept;
		static std::string		_getBody( char *, int );

		HTTPparser( void ) noexcept {};
		~HTTPparser( void ) noexcept;
		HTTPparser( HTTPparser const& ) noexcept;
		HTTPparser& operator=( HTTPparser const& ) noexcept;
};

#include "Server.hpp"