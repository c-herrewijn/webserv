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

// RequestValidate::RequestValidate(ConfigServer* conf, HTTPrequest& req)
// {
// 	if (!conf)
// 		throw RequestException({"ConfigServer pointer is NULL, unacceptable!"}, 500);
// 	this->config = conf;
// 	this->request = &req;
// 	validLocation = NULL;
// 	validParams = NULL;
// 	_autoIndex = false;
// 	solvePath();
// 	_handleStatus();
// 	_giveValidation();
// }

RequestValidate::RequestValidate(void)
{
	config = nullptr;
	validLocation = nullptr;
	validParams = nullptr;
	_autoIndex = false;
	config = nullptr;
}

// RequestValidate::RequestValidate(const RequestValidate& copy) :
// 	validLocation(copy.validLocation),
// 	validParams(copy.validParams),
// 	targetDir(copy.targetDir),
// 	targetFile(copy.targetFile),
// 	folders(copy.folders),
// 	request(copy.request),
// 	config(copy.config),
// 	statusCode(copy.statusCode),
// 	execDir(copy.execDir),
// 	_autoIndex(copy._autoIndex),
// 	cgi(copy.cgi)
// {
	
// }

// RequestValidate&	RequestValidate::operator=(const RequestValidate& assign)
// {
// 	if (this != &assign)
// 	{
// 		folders.clear();
// 		validLocation = assign.validLocation;
// 		validParams = assign.validParams;
// 		targetDir = assign.targetDir;
// 		targetFile = assign.targetFile;
// 		request = assign.request;
// 		config = assign.config;
// 		folders = assign.folders;
// 		statusCode = assign.statusCode;
// 		execDir = assign.execDir;
// 		_autoIndex = assign._autoIndex;
// 		cgi = assign.cgi;
// 	}
// 	return (*this);
// }

// RequestValidate::~RequestValidate(void)
// {

// }

// Getter functions
bool	RequestValidate::isCGI() const
{
	return (_isCGI);
}

Location* RequestValidate::_getValidLocation() const
{
	return (validLocation);
}

Parameters* RequestValidate::_getValidParams() const
{
	return (validParams);
}

bool	RequestValidate::isAutoIndex() const
{
	return ((_autoIndex));
}

const std::string& RequestValidate::_getTargetDir() const
{
	return (targetDir);
}

const std::string& RequestValidate::_getTargetFile() const
{
	return (targetFile);
}

int RequestValidate::getStatusCode() const
{
	return (statusCode);
}

t_path const&	RequestValidate::getRealPath() const
{
	execDir.lexically_normal();
	return (execDir);
}

Location*	RequestValidate::_diveLocation(Location& cur, std::vector<std::string>::iterator itDirectory)
{
	std::vector<std::string> curURL;
	std::vector<std::string>::iterator itFolders;

	_separateFolders(cur.getURL(), curURL);
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
			Location*	valid = _diveLocation(nest, itDirectory);
			if (valid)
				return (valid);
		}
	}
	return (NULL);
}

void	RequestValidate::_initValidLocation(void)
{
	Location*							valid;
	std::vector<std::string>::iterator	it;

	_separateFolders(targetDir, folders);
	for (auto location : config.getLocations())
	{
		it = folders.begin();
		valid = _diveLocation(location, it);
		if (valid)
			break ;
	}
	if (valid)
	{
		validLocation = valid;
		_setValidParams(&valid->getParams());
	}
}

void	RequestValidate::_setValidParams(const Parameters* params)
{
	validParams = const_cast<Parameters*>(params);
}

void	RequestValidate::_setTargetFile(const std::string& file)
{
	targetFile = file;
}

void	RequestValidate::_setTargetDir(const std::string& dir)
{
	targetDir = dir;
}

void	RequestValidate::_setStatusCode(const size_t& code)
{
	statusCode = code;
}

void	RequestValidate::_setAutoIndex(bool index)
{
	_autoIndex = index;
}

void	RequestValidate::_separateFolders(std::string const& input, std::vector<std::string>& output)
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

void	RequestValidate::_initTargetDir(void)
{
	if (!targetFile.empty())
	{
		targetDir = this->requestPath.parent_path().string();
		// targetDir = request->getUrl().path.parent_path().string();
		if (targetDir.back() != '/')
			targetDir += "/";
	}
	else
		targetDir =  this->requestPath.string();
}

bool	RequestValidate::_handleFolder(void)
{
	t_path dirPath = t_path(validParams->getRoot()) / targetDir;
	dirPath /= "";
	execDir = dirPath;
	if (!std::filesystem::exists(dirPath) ||
	!std::filesystem::is_directory(dirPath))
		return (_setStatusCode(404), false);// 404 error, not found
	// check autoindex
	if (!validParams->getAutoindex())
		return (_setStatusCode(404), false);// 404 error, not found
	_autoIndex = true;
	return(true);
}

bool	RequestValidate::_handleFile(void)
{
	t_path dirPath = t_path(validParams->getRoot()) / targetDir;
	dirPath /= "";
	t_path filePath = dirPath / targetFile;

	// check filePath
	if (!std::filesystem::exists(filePath) ||
		std::filesystem::is_directory(filePath))
	{
		execDir = dirPath;
		// check folderPath
		if (!std::filesystem::exists(dirPath) ||
			!std::filesystem::is_directory(dirPath))
			return (_setStatusCode(404), false);// 404 error, not found
		// check autoindex
		if (!validParams->getAutoindex())
			return (_setStatusCode(404), false);// 404 error, not found
		_autoIndex = true;
	}
	else
	{
		execDir = filePath;
		// check permission
		std::filesystem::perms permissions = std::filesystem::status(filePath).permissions();
		// check request type for permission check
		if (validParams->getCgiAllowed() &&
			filePath.has_extension() &&
			filePath.extension() == validParams->getCgiExtension())
		{
			if ((permissions & std::filesystem::perms::others_exec) == std::filesystem::perms::none)
				return (_setStatusCode(403), false);// 403 error, permission denied
			_isCGI = true;
		}
		else
		{
			if ((permissions & std::filesystem::perms::others_read) == std::filesystem::perms::none)
				return (_setStatusCode(403), false); // Handle 403 error, permissions
			// if (std::filesystem::file_size(filePath) > validParams->getMaxSize())
			// 	return (_setStatusCode(413), false);// 413 error, size
		}
	}
	return (true);
}

bool	RequestValidate::_handleReturns(void)
{
	auto it = validParams->getReturns().find(statusCode);
	if (it == validParams->getReturns().end())
		return (true);
	_isCGI = false;
	_autoIndex = false;
	t_path path = (*it).second;
	targetFile = path.filename();
	if (!targetFile.empty())
	{
		targetDir = path.parent_path().string();
		if (targetDir.back() != '/')
			targetDir += "/";
	}
	else
		targetDir = path.string();
	if (targetFile.empty() && !_handleFolder())
		return (false);
	else if (!_handleFile())
		return (false);
	return (true);
}

bool	RequestValidate::_handleErrorCode(void)
{
	auto it = validParams->getErrorPages().find(statusCode);
	if (it == validParams->getErrorPages().end())
		return (true);
	_isCGI = false;
	_autoIndex = false;
	t_path path = (*it).second;
	targetFile = path.filename();
	if (!targetFile.empty())
	{
		targetDir = path.parent_path().string();
		if (targetDir.back() != '/')
			targetDir += "/";
	}
	else
		targetDir = path.string();
	if (targetFile.empty() && !_handleFolder())
		return (false);
	else if (!_handleFile())
		return (false);
	return (true);
}

bool	RequestValidate::_handleServerPages(void)
{
	t_path lastChance = std::filesystem::current_path();
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

void	RequestValidate::_handleStatus(void)
{
	if (_handleReturns())
		return ;
	if (_handleErrorCode())
		return ;
	_setValidParams(&config.getParams());
	if (_handleErrorCode())
		return ;
	if (_handleServerPages())
		return ;
	// use servers error page
	_setStatusCode(500); // handleInternal();
}

void	RequestValidate::setConfig( ConfigServer const& configServ)
{
	this->config = configServ;
}

void	RequestValidate::setMethod( HTTPmethod method )
{
	this->requestMethod = method;
}

void	RequestValidate::setPath( t_path const& newPath )
{
	this->requestPath = newPath;
}

void	RequestValidate::solvePath(void)
{
	// given NULL is not valid
	if (!config)
		return (_setStatusCode(404));
	_setStatusCode(200);
	// default params
	_setValidParams(&config.getParams());
	// set asked file
	_setTargetFile( this->requestPath.filename());
	// save path except file
	_initTargetDir();
	// if directory is not root check for location
	if (_getTargetDir() != "/")
	{
		_initValidLocation();
		if (!_getValidLocation())
			return (_setStatusCode(404));// 404 error, not found
	}
	if (!validParams->getAllowedMethods()[requestMethod])
		return (_setStatusCode(405));// 405 error, method not allowed
	// set indexfile if necessarry
	if (_getTargetFile().empty())
		_setTargetFile(_getValidParams()->getIndex());
	if (targetFile.empty())
		_handleFolder();
	else
		_handleFile();
}