/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPresponse.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:01:20 by fra           #+#    #+#                 */
/*   Updated: 2024/02/15 18:02:32 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <ctime>

#include "HTTPstruct.hpp"

class HTTPresponse : public HTTPstruct
{
	public:
		HTTPresponse( void ) : HTTPstruct() {};
		virtual ~HTTPresponse( void ) override {};

		void		parseBody( std::string const& ) noexcept override;
		void		buildResponse( int, std::string const&, std::string const& ) noexcept;
		std::string	toString( void ) const noexcept override;

		int					getStatusCode( void ) const noexcept;
		std::string const&	getStatusStr( void ) const noexcept;

	protected:
		int         _statusCode;
		std::string _statusStr;
		std::string	_mapStatusCode( int ) const ;
		void		_setHead( std::string const& ) override;
		void		_addHeader(std::string const&, std::string const& ) noexcept;
		std::string	_getDateTime( void ) const noexcept;
};