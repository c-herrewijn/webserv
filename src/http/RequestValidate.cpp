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
	if (!conf)
		throw ExecException({"ConfigServer pointer is NULL, unacceptable!"});
	this->config = conf;
	this->request = &req;
	validLocation = NULL;
	validParams = NULL;
	autoIndex = false;
	initElements();
	handleStatus();
}

RequestValidate::RequestValidate(void)
{
	request = NULL;
	config = NULL;
	validLocation = NULL;
	validParams = NULL;
	autoIndex = false;
}

RequestValidate::RequestValidate(const RequestValidate& copy) :
	request(copy.request),
	config(copy.config),
	validLocation(copy.validLocation),
	validParams(copy.validParams),
	autoIndex(copy.autoIndex),
	cgi(copy.cgi),
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
		autoIndex = assign.autoIndex;
		cgi = assign.cgi;
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

bool	RequestValidate::getAutoIndex() const
{
	return (autoIndex);
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


std::filesystem::path	RequestValidate::getTargetPath() const
{
	return (std::filesystem::path(targetDir + "/" + targetFile));
}

const std::vector<std::string>& RequestValidate::getFolders() const
{
	return folders;
}

// Setter functions
void	RequestValidate::setRequest(HTTPrequest* req)
{
	if (!req)
		throw ExecException({"HTTPrequest pointer is NULL, unacceptable!"});
	request = req;
	initElements();
	handleStatus();
}

void	RequestValidate::setConfig(ConfigServer* conf)
{
	config = conf;
	if (!conf)
		throw ExecException({"ConfigServer pointer is NULL, unacceptable!"});
	initElements();
	handleStatus();
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

void	RequestValidate::setAutoIndex(bool index)
{
	autoIndex = index;
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

bool	RequestValidate::handleFolder(void)
{
	std::filesystem::path dirPath = std::filesystem::path(validParams->getRoot()) / targetDir;
	execDir = dirPath;
	if (!std::filesystem::exists(dirPath) ||
	!std::filesystem::is_directory(dirPath))
		return (setStatusCode(404), false);// 404 error, not found
	// check autoindex
	if (!validParams->getAutoindex())
		return (setStatusCode(404), false);// 404 error, not found
	autoIndex = true;
	return(true);
}

bool	RequestValidate::handleFile(void)
{
	std::filesystem::path dirPath = std::filesystem::path(validParams->getRoot()) / targetDir;
	std::filesystem::path filePath = dirPath / targetFile;

	// check filePath
	if (!std::filesystem::exists(filePath) ||
		std::filesystem::is_directory(filePath))
	{
		execDir = dirPath;
		// check folderPath
		if (!std::filesystem::exists(dirPath) ||
			!std::filesystem::is_directory(dirPath))
			return (setStatusCode(404), false);// 404 error, not found
		// check autoindex
		if (!validParams->getAutoindex())
			return (setStatusCode(404), false);// 404 error, not found
		autoIndex = true;
	}
	else
	{
		execDir = filePath;
		// check permission
		std::filesystem::perms permissions = std::filesystem::status(filePath).permissions();
		// check request type for permission check
		if (config->getCgiAllowed() &&
			filePath.has_extension() &&
			filePath.extension() == config->getCgiExtension())
		{
			if ((permissions & std::filesystem::perms::others_exec) == std::filesystem::perms::none)
				return (setStatusCode(403), false);// 403 error, permission denied
			cgi = true;
		}
		else
		{
			if ((permissions & std::filesystem::perms::others_read) == std::filesystem::perms::none)
				return (setStatusCode(403), false); // Handle 403 error, permissions
			if (std::filesystem::file_size(filePath) > validParams->getMaxSize())
				return (setStatusCode(413), false);// 413 error, size
		}
	}
	return (true);
}

bool	RequestValidate::handleReturns(void)
{
	auto it = validParams->getReturns().find(statusCode);
	if (it == validParams->getReturns().end())
		return (true);
	cgi = false;
	autoIndex = false;
	std::filesystem::path path = (*it).second;
	targetFile = path.filename();
	if (!targetFile.empty())
	{
		targetDir = path.parent_path().string();
		if (targetDir.back() != '/')
			targetDir += "/";
	}
	else
		targetDir = path.string();
	if (targetFile.empty() && !handleFolder())
		return (false);
	else if (!handleFile())
		return (false);
	return (true);
}

bool	RequestValidate::handleErrorCode(void)
{
	auto it = validParams->getErrorPages().find(statusCode);
	if (it == validParams->getErrorPages().end())
		return (true);
	cgi = false;
	autoIndex = false;
	std::filesystem::path path = (*it).second;
	targetFile = path.filename();
	if (!targetFile.empty())
	{
		targetDir = path.parent_path().string();
		if (targetDir.back() != '/')
			targetDir += "/";
	}
	else
		targetDir = path.string();
	if (targetFile.empty() && !handleFolder())
		return (false);
	else if (!handleFile())
		return (false);
	return (true);
}

bool	RequestValidate::handleServerPages(void)
{
	std::filesystem::path lastChance = std::filesystem::current_path();
	lastChance /= "default";
	lastChance /= "pages";
	lastChance /= std::to_string(statusCode) + ".html";
	execDir = lastChance;
	if (!std::filesystem::exists(lastChance) ||
		std::filesystem::is_directory(lastChance))
		return (false);
	std::filesystem::perms permissions = std::filesystem::status(lastChance).permissions();
	if ((permissions & std::filesystem::perms::others_read) == std::filesystem::perms::none)
		return (false);
	targetDir = lastChance.parent_path();
	targetFile = lastChance.filename();
	return (true);
}

void	RequestValidate::handleStatus(void)
{
	if (handleReturns())
		return ;
	if (handleErrorCode())
		return ;
	setValidParams(&config->getParams());
	if (handleErrorCode())
		return ;
	if (handleServerPages())
		return ;
	// use servers error page
	setStatusCode(500); // handleInternal();
}

void	RequestValidate::initElements(void)
{
	// given NULL is not valid
	if (!request || !config)
		return (setStatusCode(404));
	setStatusCode(200);
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
			return (setStatusCode(404));// 404 error, not found
	}
	if (!validParams->getAllowedMethods()[request->getMethod()])
		return (setStatusCode(405));// 405 error, method not allowed
	// set indexfile if necessarry
	if (getTargetFile().empty())
		setTargetFile(getValidParams()->getIndex());
	if (targetFile.empty())
		handleFolder();
	else
		handleFile();
}
