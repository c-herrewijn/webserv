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
# include "HTTPrequest.hpp"
# include "ConfigServer.hpp"
// Needed for file size calculation
# include <iostream>
# include <fstream>
// filesystem management
# include <filesystem>
# include "Exceptions.hpp"

class RequestValidate
{
	private:
		Location*					validLocation;
		Parameters*					validParams;
		std::string					targetDir;
		std::string					targetFile;
		std::vector<std::string>	folders;

		HTTPrequest*				request;
		ConfigServer*				config;
		size_t						statusCode;
		std::filesystem::path		execDir;
		bool						autoIndex;
		bool						cgi;

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

		void		_initElements(void);
		void		_initValidLocation(void);
		void		_initTargetDir(void);

		bool		_handleFolder(void);
		bool		_handleFile(void);
		void		_handleStatus(void);
		bool		_handleReturns(void);
		bool		_handleErrorCode(void);
		bool		_handleServerPages(void);
	public:
		RequestValidate(ConfigServer* conf, HTTPrequest& req);
		RequestValidate(void);
		RequestValidate(const RequestValidate& copy);
		RequestValidate&	operator=(const RequestValidate& assign);
		virtual ~RequestValidate(void);

		HTTPrequest*			getRequest() const;
		ConfigServer*			getConfig() const;
		bool					getCGI() const;
		bool					getAutoIndex() const;
		size_t					getStatusCode() const;
		std::filesystem::path	getExecPath() const;

		void setRequest(HTTPrequest* req);
		void setConfig(ConfigServer* conf);
};

#endif