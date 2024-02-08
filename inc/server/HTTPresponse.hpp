/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPresponse.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/02/08 21:01:20 by fra           #+#    #+#                 */
/*   Updated: 2024/02/09 00:30:15 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <ctime>
#include "HTTPstruct.hpp"

class HTTPresponse : public HTTPstruct
{
	public:
		HTTPresponse( std::string const& strHeads="", std::string const& strBody="" );
		virtual ~HTTPresponse( void ) override {};

		void		parseBody( std::string const& ) override;
		// void	buildResponse( int code, std::string const& body );
		std::string	toString( void ) const override;


	protected:
		int         _statusCode;
		std::string _statusStr;

		void		_setHead( std::string const& ) override;
		void		_setHead( int, std::string const& );
		void		_addHeader(std::string const&, std::string const& );
		std::string	_mapStatus( int ) const;
		std::string	_getDateTime( void ) const;
};