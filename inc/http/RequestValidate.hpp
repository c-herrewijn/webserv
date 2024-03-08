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
// # include <filesystem>
# include "Exceptions.hpp"

class RequestValidate
{
	private:
		HTTPmethod					requestMethod;
		t_path						requestPath;

		t_path						execPath;
		bool						_autoIndex;
		bool						_isCGI;
		bool						_isFile;
		

		Location*					validLocation;
		Parameters*					validParams;
		std::string					targetDir;
		std::string					targetFile;
		std::vector<std::string>	folders;

		ConfigServer const*			config;
		size_t						statusCode;
		t_path						execDir;

		Location*			_getValidLocation() const;
		Parameters*			_getValidParams() const;
		const std::string&	_getTargetDir() const;
		const std::string&	_getTargetFile() const;

		void		_setTargetFile(const std::string& file);
		void		_setValidParams(const Parameters* params);
		void		_setTargetDir(const std::string& dir);
		void		_setStatusCode(const size_t& code);
		void		_setAutoIndex(bool index);

		void		_separateFolders(std::string const& input, std::vector<std::string>& output);
		Location*	_diveLocation(Location& cur, std::vector<std::string>::iterator itDirectory);

		void		_initValidLocation(void);
		void		_initTargetDir(void);

		bool		_handleFolder(void);
		bool		_handleFile(void);
		void		_handleStatus(void);
		bool		_handleReturns(void);
		bool		_handleErrorCode(void);
		bool		_handleServerPages(void);

	public:
		RequestValidate(void);

		void	setConfig( ConfigServer const* );
		void	setMethod( HTTPmethod );
		void	setPath( t_path const& );
	
		t_path const&	getExecPath( void ) const;
		int				getStatusCode( void ) const;
		bool			isCGI() const;
		bool			isAutoIndex() const;
		bool			isFile() const;
		void			solvePath( void );
		virtual 		~RequestValidate(void) {};
};

#endif