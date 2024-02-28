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

#include "RequestValidate.hpp"

RequestValidate::RequestValidate(ConfigServer* conf, HTTPrequest& req)
{
	this->config = conf;
	this->request = &req;
	validLocation = NULL;
	validParams = NULL;
	initElements();
	// check return, error_page, default error_page, error_page creation
}

RequestValidate::RequestValidate(void)
{
	request = NULL;
	config = NULL;
	validLocation = NULL;
	validParams = NULL;
}

RequestValidate::RequestValidate(const RequestValidate& copy) :
	request(copy.request),
	config(copy.config),
	validLocation(copy.validLocation),
	validParams(copy.validParams),
	targetDir(copy.targetDir),
	targetFile(copy.targetFile),
	statusCode(copy.statusCode),
	folders(copy.folders)
{
	
}

RequestValidate&	RequestValidate::operator=(const RequestValidate& assign)
{
	if (this != &assign)
	{
		folders.clear();
		request = assign.request;
		config = assign.config;
		validLocation = assign.validLocation;
		validParams = assign.validParams;
		targetDir = assign.targetDir;
		targetFile = assign.targetFile;
		statusCode = assign.statusCode;
		folders = assign.folders;
	}
	return (*this);
}

RequestValidate::~RequestValidate(void)
{

}

// Getter functions
HTTPrequest* RequestValidate::getRequest() const
{
	return request;
}

ConfigServer* RequestValidate::getConfig() const
{
	return config;
}

Location* RequestValidate::getValidLocation() const
{
	return validLocation;
}

Parameters* RequestValidate::getValidParams() const
{
	return validParams;
}

const std::string& RequestValidate::getTargetDir() const
{
	return targetDir;
}

const std::string& RequestValidate::getTargetFile() const
{
	return targetFile;
}

size_t RequestValidate::getStatusCode() const
{
	return statusCode;
}

const std::vector<std::string>& RequestValidate::getFolders() const
{
	return folders;
}

// Setter functions
void	RequestValidate::setRequest(HTTPrequest* req)
{
	request = req;
	initElements();
}

void	RequestValidate::setConfig(ConfigServer* conf)
{
	config = conf;
	initElements();
}

Location*	RequestValidate::diveLocation(Location& cur, std::vector<std::string>::iterator itDirectory)
{
	std::vector<std::string> curURL;
	std::vector<std::string>::iterator itFolders;

	separateFolders(cur.getURL(), curURL);
	itFolders = curURL.begin();
	while (itFolders != curURL.end() && itDirectory != folders.end())
	{
		if (*itFolders != *itDirectory)
			break ;
		itFolders++;
		itDirectory++;
	}
	if (itFolders == curURL.end() && itDirectory == folders.end())
		return (&cur);
	else if (itFolders == curURL.end())
	{
		for (auto nest : cur.getNested())
		{
			Location*	valid = diveLocation(nest, itDirectory);
			if (valid)
				return (valid);
		}
	}
	return (NULL);
}

void	RequestValidate::initValidLocation(void)
{
	Location*							valid;
	std::vector<std::string>::iterator	it;

	separateFolders(targetDir, folders);
	for (auto location : config->getLocations())
	{
		it = folders.begin();
		valid = diveLocation(location, it);
		if (valid)
			break ;
	}
	if (valid)
	{
		validLocation = valid;
		setValidParams(&valid->getParams());
	}
}

void	RequestValidate::setValidParams(const Parameters* params)
{
	validParams = const_cast<Parameters*>(params);
}

void	RequestValidate::setTargetFile(const std::string& file)
{
	targetFile = file;
}

void	RequestValidate::setTargetDir(const std::string& dir)
{
	targetDir = dir;
}

void	RequestValidate::setStatusCode(const size_t& code)
{
	statusCode = code;
}

void	RequestValidate::separateFolders(std::string const& input, std::vector<std::string>& output)
{
	std::string	tmp;

	output.clear();
	for (auto letter : input)
	{
		if (letter == '/' && !tmp.empty())
		{
			output.push_back(tmp);
			tmp.clear();
		}
		else
			tmp += letter;
	}
}

void	RequestValidate::initTargetDir(void)
{
	if (!targetFile.empty())
	{
		targetDir = request->getUrl().path.parent_path().string();
		if (targetDir.back() != '/')
			targetDir += "/";
	}
	else
		targetDir = request->getUrl().path.string();
}

// void	RequestValidate::findAndError()

void	RequestValidate::initElements(void)
{
	// given NULL is not valid
	if (!request || !config)
		return ;
	// default params
	setValidParams(&config->getParams());
	// set asked file
	setTargetFile(request->getUrl().path.filename());
	// save path except file
	initTargetDir();
	// if directory is not root check for location
	if (getTargetDir() != "/")
	{
		initValidLocation();
		if (!getValidLocation())
			return ;// 404 error, not found
	}
	if (!validParams->getAllowedMethods()[request->getMethod()])
		return ;// 405 error, method not allowed
	// set if indexfile is necessarry
	if (getTargetFile().empty())
		setTargetFile(getValidParams()->getIndex());
	// check folder existance
	std::filesystem::path dirPath = std::filesystem::path(validParams->getRoot()) / targetDir;
	std::filesystem::path filePath = dirPath / targetFile;
	if (!std::filesystem::exists(dirPath) ||
		!std::filesystem::is_directory(dirPath))
		return ;// 404 error, not found
	if (!std::filesystem::exists(filePath) ||
		std::filesystem::is_directory(filePath))
	{
		// check autoindex
		if (!validParams->getAutoindex())
			return ;// 404 error, not found
		// autoindex behaviour???
	}
	else
	{
		// check permission
		std::filesystem::perms permissions = std::filesystem::status(filePath).permissions();
		if ((permissions & std::filesystem::perms::others_read) == std::filesystem::perms::none) {
			// Handle 403 error, permissions
			return;
		}
		// check size
		if (std::filesystem::file_size(filePath) > validParams->getMaxSize())
			return ;// 431 error, size
	}
	return ;// success
}
