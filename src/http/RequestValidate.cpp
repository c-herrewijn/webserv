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

// ╔════════════════════════════════╗
// ║		CONSTRUCTION PART		║
// ╚════════════════════════════════╝
RequestValidate::RequestValidate(void)
{
	_requestConfig = nullptr;
	_validLocation = nullptr;
	_validParams = nullptr;

	_isCGI = false;
	_isFile = false;
	_autoIndex = false;
}

RequestValidate::~RequestValidate( void ) {}

// ╔════════════════════════════════╗
// ║			GETTER PART			║
// ╚════════════════════════════════╝
Parameters*	RequestValidate::getValidParams( void ) const
{
	return (_validParams);
}

t_path const&	RequestValidate::getExecPath( void ) const
{
	return (_execPath);
}

int	RequestValidate::getStatusCode( void ) const
{
	return (_statusCode);
}

bool	RequestValidate::isAutoIndex( void ) const
{
	return (_autoIndex);
}

bool	RequestValidate::isFile( void ) const
{
	return (_isFile);
}

bool	RequestValidate::isCGI( void ) const
{
	return (_isCGI);
}

// ╔════════════════════════════════╗
// ║			SETTER PART			║
// ╚════════════════════════════════╝
void	RequestValidate::setConfig( ConfigServer const* configServ)
{
	this->_requestConfig = configServ;
}

void	RequestValidate::setMethod( HTTPmethod method )
{
	this->_requestMethod = method;
}

void	RequestValidate::setPath( t_path const& newPath )
{
	this->_requestPath = newPath;
}

void	RequestValidate::_setStatusCode(const size_t& code)
{
	_statusCode = code;
}

// ╔════════════════════════════════╗
// ║			SOLVING PART		║
// ╚════════════════════════════════╝
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

	_separateFolders(targetDir.string(), folders);
	for (auto location : _requestConfig->getLocations())
	{
		it = folders.begin();
		valid = _diveLocation(location, it);
		if (valid)
			break ;
	}
	if (valid)
	{
		_validLocation = valid;
		_validParams = const_cast<Parameters*>(&valid->getParams());
	}
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
	if (!targetFile.string().empty())
	{
		targetDir = this->_requestPath.parent_path();
		if (targetDir.string().back() != '/')
			targetDir += "/";
	}
	else
		targetDir =  this->_requestPath;
}

bool	RequestValidate::_handleFolder(void)
{
	t_path dirPath = t_path(_validParams->getRoot()) / targetDir;
	dirPath /= "";
	_execPath = dirPath;
	if (!std::filesystem::exists(dirPath) ||
	!std::filesystem::is_directory(dirPath))
		return (_setStatusCode(404), false);// 404 error, not found
	// check autoindex
	if (!_validParams->getAutoindex())
		return (_setStatusCode(404), false);// 404 error, not found
	_autoIndex = true;
	return(true);
}

bool	RequestValidate::_handleFile(void)
{
	t_path dirPath = t_path(_validParams->getRoot()) / targetDir;
	dirPath /= "";
	t_path filePath = dirPath / targetFile;

	// check filePath
	if (!std::filesystem::exists(filePath) ||
		std::filesystem::is_directory(filePath))
	{
		_execPath = dirPath;
		// check folderPath
		if (!std::filesystem::exists(dirPath) ||
			!std::filesystem::is_directory(dirPath))
			return (_setStatusCode(404), false);// 404 error, not found
		// check autoindex
		if (!_validParams->getAutoindex())
			return (_setStatusCode(404), false);// 404 error, not found
		_autoIndex = true;
	}
	else
	{
		_execPath = filePath;
		// check permission
		std::filesystem::perms permissions = std::filesystem::status(filePath).permissions();
		// check request type for permission check
		if (_validParams->getCgiAllowed() &&
			filePath.has_extension() &&
			filePath.extension() == _validParams->getCgiExtension())
		{
			if ((permissions & std::filesystem::perms::others_exec) == std::filesystem::perms::none)
				return (_setStatusCode(403), false);// 403 error, permission denied
			_isCGI = true;
		}
		else
		{
			if ((permissions & std::filesystem::perms::others_read) == std::filesystem::perms::none)
				return (_setStatusCode(403), false); // Handle 403 error, permissions
			// if (std::filesystem::file_size(filePath) > _validParams->getMaxSize())
			// 	return (_setStatusCode(413), false);// 413 error, size
		}
	}
	return (true);
}

bool	RequestValidate::_handleReturns(void)
{
	auto it = _validParams->getReturns().find(_statusCode);
	if (it == _validParams->getReturns().end())
		return (true);
	_isCGI = false;
	_autoIndex = false;
	t_path path = (*it).second;
	targetFile = path.filename();
	if (!targetFile.empty())
	{
		targetDir = path.parent_path();
		if (targetDir.string().back() != '/')
			targetDir += "/";
	}
	else
		targetDir = path.string();
	if (targetFile.string().empty() && !_handleFolder())
		return (false);
	else if (!_handleFile())
		return (false);
	return (true);
}

bool	RequestValidate::_handleErrorCode(void)
{
	auto it = _validParams->getErrorPages().find(_statusCode);
	if (it == _validParams->getErrorPages().end())
		return (true);
	_isCGI = false;
	_autoIndex = false;
	t_path path = (*it).second;
	targetFile = path.filename().string();
	if (!targetFile.string().empty())
	{
		targetDir = path.parent_path();
		if (targetDir.string().back() != '/')
			targetDir += "/";
	}
	else
		targetDir = path.string();
	if (targetFile.string().empty() && !_handleFolder())
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
	lastChance /= std::to_string(_statusCode) + ".html";
	_execPath = lastChance;
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
	_validParams = const_cast<Parameters*>(&_requestConfig->getParams());
	if (_handleErrorCode())
		return ;
	if (_handleServerPages())
		return ;
	// use servers error page
	_setStatusCode(500); // handleInternal();
}

void	RequestValidate::solvePath(void)
{
	// given NULL is not valid
	if (!_requestConfig)
		throw(RequestException({"No config given. Validation can not continue."}, 500));
	_setStatusCode(200);
	// default params
	_validParams = const_cast<Parameters*>(&_requestConfig->getParams());
	// set asked file
	t_path	test = "////////";
	std::cout << test.string() << std::endl;
	targetFile = _requestPath.filename();
	// save path except file
	_initTargetDir();
	// if directory is not root check for location
	if (targetDir.string() != "/")
	{
		_initValidLocation();
		if (_validLocation == nullptr)
			return (_setStatusCode(404));// 404 error, not found
	}
	if (!_validParams->getAllowedMethods()[_requestMethod])
		return (_setStatusCode(405));// 405 error, method not allowed
	// set indexfile if necessarry
	if (targetFile.string().empty())
		targetFile = _validParams->getIndex();
	if (targetFile.string().empty())
		_handleFolder();
	else
		_handleFile();
}