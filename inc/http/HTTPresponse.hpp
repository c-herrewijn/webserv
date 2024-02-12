/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPresponse.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:01:20 by fra           #+#    #+#                 */
/*   Updated: 2024/02/12 08:34:58 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <ctime>

#include "HTTPstruct.hpp"

class HTTPresponse : public HTTPstruct
{
	public:
		HTTPresponse( void );
		virtual ~HTTPresponse( void ) override {};

		void		parseBody( std::string const& ) override;
		void		buildResponse( int, std::string const&, std::string const& ) noexcept;
		std::string	toString( void ) const override;

		int					getStatusCode( void ) const;
		std::string const&	getStatusStr( void ) const;

	protected:
		int         _statusCode;
		std::string _statusStr;

		void		_setHead( std::string const& ) override;
		void		_addHeader(std::string const&, std::string const& );
		std::string	_mapStatus( int ) const;
		std::string	_getDateTime( void ) const;
};