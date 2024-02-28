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

class RequestValidate
{
	private:
		HTTPrequest*				request;
		ConfigServer*				config;
		Location*					validLocation;
		Parameters*					validParams;
		std::string					targetDir;
		std::string					targetFile;
		size_t						statusCode;
		std::vector<std::string>	folders;

		void		setTargetFile(const std::string& file);
		void		setValidParams(const Parameters* params);
		void		setTargetDir(const std::string& dir);
		void		setStatusCode(const size_t& code);
		void		separateFolders(std::string const& input, std::vector<std::string>& output);
		Location*	diveLocation(Location& cur, std::vector<std::string>::iterator itDirectory);
		void		initElements(void);
		void		initValidLocation(void);
		void		initTargetDir(void);
	public:
		RequestValidate(ConfigServer* conf, HTTPrequest& req);
		RequestValidate(void);
		RequestValidate(const RequestValidate& copy);
		RequestValidate&	operator=(const RequestValidate& assign);
		virtual ~RequestValidate(void);

		// Getter functions
		HTTPrequest*					getRequest() const;
		ConfigServer*					getConfig() const;
		Location*						getValidLocation() const;
		Parameters*						getValidParams() const;
		const std::string&				getTargetDir() const;
		const std::string&				getTargetFile() const;
		size_t							getStatusCode() const;
		const std::vector<std::string>&	getFolders() const;

		// Setter functions
		void setRequest(HTTPrequest* req);
		void setConfig(ConfigServer* conf);
};

#endif