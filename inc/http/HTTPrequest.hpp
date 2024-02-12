/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPrequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 17:05:42 by faru          #+#    #+#                 */
/*   Updated: 2024/02/12 12:13:15 by faru          ########   odam.nl         */
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
		HTTPrequest( void );
		virtual ~HTTPrequest( void ) override {};

		void					setTmpBody( std::string const& );
		std::string const&		getTmpBody( void ) const;

		void		parseBody( std::string const& ) override;
		std::string	toString( void ) const override;
		std::string	getHost( void ) const ;

	protected:
		HTTPmethod	_method;
		HTTPurl		_url;
		std::string	_tmpBody;

		void	_setHead( std::string const& ) override;
		void	_setHeaders( std::string const& ) override;
		void	_setBody( std::string const& ) override;

		void	_setMethod( std::string const& );
		void	_setURL( std::string const& );
		void	_setVersion( std::string const& );

		void	_setScheme( std::string const& );
		void	_setHostPort( std::string const& );
		void	_setPath( std::string const& );
		void	_setQuery( std::string const& );
		void	_setFragment( std::string const& );
		
		void	_setChunkedBody( std::string const& );
};