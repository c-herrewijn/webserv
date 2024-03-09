/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestValidate.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 11:44:46 by itopchu       #+#    #+#                 */
/*   Updated: 2023/11/25 11:44:46 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTVALIDATE_HPP
# define REQUESTVALIDATE_HPP
# include "HTTPstruct.hpp"
# include "ConfigServer.hpp"
// Needed for file size calculation
# include <iostream>
# include <fstream>
// filesystem management
# include "Exceptions.hpp"

class RequestValidate
{
	private:
		ConfigServer const*			_requestConfig;
		HTTPmethod					_requestMethod;
		t_path						_requestPath;

		ConfigServer				config;
		t_path						execDir;
		t_path						_realPath;
		size_t						_maxBodySize;
		bool						_autoIndex;
		bool						_isCGI;
		bool						_isFile;

		Location*					_validLocation;
		Parameters*					_validParams;
		t_path						targetDir;
		t_path						targetFile;
		std::vector<std::string>	folders;

		size_t						_statusCode;

		void		_setStatusCode(const size_t& code);

		void		_separateFolders(std::string const& input, std::vector<std::string>& output);
		Location*	_diveLocation(Location& cur, std::vector<std::string>::iterator itDirectory);

		void		_initValidLocation( void );
		void		_initTargetDir( void );

		bool		_handleFolder( void );
		bool		_handleFile( void );
		void		_handleStatus( void );
		bool		_handleReturns( void );
		bool		_handleErrorCode( void );
		bool		_handleServerPages( void );

	public:
		RequestValidate( void );
		virtual	~RequestValidate( void );

		void	setConfig( ConfigServer const& );
		void	setMethod( HTTPmethod );
		void	setPath( t_path const& );
		void	solvePath( void );

		Parameters*		getValidParams( void ) const;
		t_path const&	getRealPath( void ) const;
		int				getStatusCode( void ) const;
		int				getMaxBodySize( void ) const;
		bool			isAutoIndex( void ) const;
		bool			isFile( void ) const;
		bool			isCGI( void ) const;
};

#endif