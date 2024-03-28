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

# define SERVER_DEF_PAGES "/default/errors/";

typedef std::filesystem::perms t_perms;

typedef enum PermType_s
{
	PERM_READ,
	PERM_WRITE,
	PERM_EXEC,
} PermType;

class RequestValidate
{
	private:
		ConfigServer const*	_requestConfig;
		HTTPmethod			_requestMethod;
		t_path				_requestPath;

		size_t				_statusCode;
		t_path				_realPath;
		bool				_autoIndex;
		bool				_isCGI;

		Location const*		_validLocation;
		Parameters const*	_validParams;

		// t_path				targetRoot;
		t_path				targetDir;
		t_path				targetFile;


		bool			_checkPerm(t_path const& path, PermType type);
		void			_separateFolders(std::string const& input, std::vector<std::string>& output);
		Location const*	_diveLocation(Location const& cur, std::vector<std::string>::iterator itDirectory, std::vector<std::string>& folders);

		void		_initValidLocation( void );
		void		_initTargetElements( void );

		bool	_handleFolder( void );
		bool	_handleFile( void );
		void	_handleStatus( void );
		bool	_handleReturns( void );
		bool	_handleErrorCode( void );
		bool	_handleServerPages( void );

		void		_setStatusCode(const size_t& code);

	public:
		RequestValidate( void );
		virtual	~RequestValidate( void );

		void	setConfig( ConfigServer const& );
		void	setMethod( HTTPmethod );
		void	setPath( t_path const& );
		void	solvePath( void );

		// Parameters*			getValidParams( void ) const;
		t_path const&		getRealPath( void ) const;
		void				setRealPath( t_path const& ) noexcept;
		std::uintmax_t		getMaxBodySize( void ) const;
		int					getStatusCode( void ) const;
		t_path const&		getRoot( void ) const;
		t_string_map const&	getErrorPages( void ) const;
		bool				isAutoIndex( void ) const;
		bool				isFile( void ) const;
		bool				isCGI( void ) const;
};

#endif
