/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 17:05:42 by faru          #+#    #+#                 */
/*   Updated: 2024/02/09 00:18:31 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "HTTPstruct.hpp"

typedef enum HTTPmethod_s
{
	HTTP_GET,
	HTTP_POST,
	HTTP_DELETE,
} HTTPmethod;

typedef struct HTTPurl_f
{
	std::string	scheme;
	std::string	host;
	int			port;
	std::string	path;
	dict		query;
	std::string queryRaw;
	std::string fragment;

	std::string	toString( void ) const;
} HTTPurl;

class HTTPrequest : public HTTPstruct
{
	public:
		HTTPrequest( std::string const& strHeads="", std::string const& strBody="" );
		virtual ~HTTPrequest( void ) override {};

		void		parseBody( std::string const& ) override;
		std::string	toString( void ) const override;

	protected:
		HTTPmethod	_method;
		HTTPurl		_url;

		void	_setHead( std::string const&) override;
		void	_setHeaders( std::string const&) override;

		void	_setMethod( std::string const& );
		void	_setURL( std::string const& );
		void	_setScheme( std::string const& );
		void	_setHostPort( std::string const& );
		void	_setPath( std::string const& );
		void	_setQuery( std::string const& );
		void	_setFragment( std::string const& );

		void	_setChunkedBody( std::string const& );
};