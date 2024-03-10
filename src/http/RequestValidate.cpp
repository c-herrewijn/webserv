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

t_path const&	RequestValidate::getRealPath( void ) const
{
	return (_realPath);
}

int	RequestValidate::getStatusCode( void ) const
{
	return (_statusCode);
}

std::uintmax_t	RequestValidate::getMaxBodySize( void ) const
{
	return (_validParams->getMaxSize());
}

bool	RequestValidate::isAutoIndex( void ) const
{
	return (_autoIndex);
}

bool	RequestValidate::isFile( void ) const
{
	return (_realPath.has_filename());
}

bool	RequestValidate::isCGI( void ) const
{
	return (_isCGI);
}

t_path	RequestValidate::getRoot( void ) const
{
	return (t_path(this->_validParams->getRoot()));
}

std::unordered_map<size_t, std::string>	RequestValidate::getErrPages( void ) const
{
	return (this->_validParams->getErrorPages());
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
// ╭───────────────────────────╮
// │ RECURSIVE LOCATION SEARCH │
// ╰───────────────────────────╯
Location*	RequestValidate::_diveLocation(Location& cur, std::vector<std::string>::iterator itDirectory, std::vector<std::string>& folders)
{
	std::vector<std::string> curURL;
	std::vector<std::string>::iterator itFolders;

	_separateFolders(cur.getFilesystem().string(), curURL);
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
			Location*	valid = _diveLocation(nest, itDirectory, folders);
			if (valid)
				return (valid);
		}
	}
	return (NULL);
}

void	RequestValidate::_initValidLocation(void)
{
	Location*							valid;
	std::vector<std::string>			folders;
	std::vector<std::string>::iterator	it;

	_separateFolders(targetDir.string(), folders);
	for (auto location : _requestConfig->getLocations())
	{
		it = folders.begin();
		valid = _diveLocation(location, it, folders);
		if (valid)
			break ;
	}
	if (!valid)
		return ;
	_validLocation = valid;
	_validParams = const_cast<Parameters*>(&valid->getParams());
}

void	RequestValidate::_separateFolders(std::string const& input, std::vector<std::string>& output)
{
    std::istringstream iss(input);
    std::string folder;
    while (std::getline(iss, folder, '/'))
	{
        if (!folder.empty())
            output.push_back(folder);
    }
}

// ╭───────────────────────────╮
// │     ASSIGN BASIC PATHS    │
// ╰───────────────────────────╯
void	RequestValidate::_initTargetElements(void)
{
	_requestPath = std::filesystem::weakly_canonical(_requestPath);
	if (_requestPath.has_filename())
	{
		targetDir = _requestPath.parent_path();
		targetFile = _requestPath.filename();
	}
	else
	{
		targetDir = _requestPath;
		targetFile = "";
	}
}

// ╭───────────────────────────╮
// │GENERAL CHECK FOR THE PATH │
// ╰───────────────────────────╯
bool	RequestValidate::_handleFolder(void)
{
	t_path dirPath = t_path(_validParams->getRoot()) / targetDir;
	dirPath /= "";
	dirPath = std::filesystem::weakly_canonical(dirPath);
	_realPath = dirPath;
	_autoIndex = false;
	if (!std::filesystem::exists(dirPath) ||
	!std::filesystem::is_directory(dirPath))
		return (_setStatusCode(404), false); // 404 error, not found
	// check autoindex
	if (!_validParams->getAutoindex())
		return (_setStatusCode(404), false); // 404 error, not found
	std::filesystem::perms permissions = std::filesystem::status(dirPath).permissions();
	if ((permissions & std::filesystem::perms::others_read) == std::filesystem::perms::none)
		return (_setStatusCode(403), false); // Handle 403 error, permissions
	_autoIndex = true;
	return(true);
}

bool	RequestValidate::_handleFile(void)
{
	t_path dirPath = t_path(_validParams->getRoot()) / targetDir;
	t_path filePath = dirPath / targetFile;
	_isCGI = false;
	// check filePath
	if (!std::filesystem::is_regular_file(filePath))
	{
		_realPath = std::filesystem::weakly_canonical(dirPath);
		return (_handleFolder());
	}
	_realPath = std::filesystem::weakly_canonical(filePath);
	// check permission
	std::error_code ec;
	std::filesystem::perms permissions = std::filesystem::status(filePath, ec).permissions();
	if (ec.value())
		return (_setStatusCode(403), false);
	// Check request type for permission check
	if (_validParams->getCgiAllowed() &&
		filePath.has_extension() &&
		filePath.extension() == _validParams->getCgiExtension()) {
		if ((permissions & std::filesystem::perms::others_exec) == std::filesystem::perms::none)
			return _setStatusCode(403), false; // 403 error, permission denied
		_isCGI = true;
	}
	else if ((permissions & std::filesystem::perms::others_read) == std::filesystem::perms::none)
		return _setStatusCode(403), false; // 403 error, permission denied
	return (true);
}

// ╭───────────────────────────╮
// │  STATUS CODE REDIRECTION  │
// ╰───────────────────────────╯
bool	RequestValidate::_handleReturns(void)
{
	auto it = _validParams->getReturns().find(_statusCode);
	if (it != _validParams->getReturns().end())
	{
		targetFile = std::filesystem::weakly_canonical((*it).second);
		return (_handleFile());
	}
	return (false);
}

bool	RequestValidate::_handleErrorCode(void)
{
	auto it = _validParams->getErrorPages().find(_statusCode);
	if (it != _validParams->getErrorPages().end())
	{
		targetFile = std::filesystem::weakly_canonical((*it).second);
		return (_handleFile());
	}
	return (false);
}

bool	RequestValidate::_handleServerPages(void)
{
	t_path lastChance = std::filesystem::current_path() / "default" / "pages" / (std::to_string(_statusCode) + ".html");
	if (!std::filesystem::is_regular_file(lastChance))
		return (false);
	std::error_code ec;
	std::filesystem::perms permissions = std::filesystem::status(lastChance, ec).permissions();
	if (ec.value())
		return (false);
	if ((permissions & std::filesystem::perms::others_read) == std::filesystem::perms::none)
		return (false);
	_realPath = std::filesystem::weakly_canonical(lastChance);
	return (true);
}

void	RequestValidate::_handleStatus(void)
{
	if (_handleReturns() || _statusCode == 200)
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

// ╭───────────────────────────╮
// │   MAIN FUNCTION TO START  │
// ╰───────────────────────────╯
void	RequestValidate::solvePath(void)
{
	// given NULL is not valid
	if (!_requestConfig)
		throw(RequestException({"No config given. Request validation can not continue."}, 500));
	if (_requestPath.empty())
		throw(RequestException({"No Path given. Request validation can not continue."}, 500));
	if (_requestConfig == nullptr)
		throw(RequestException({"No Config given. Request validation can not continue."}, 500));
	_setStatusCode(200);
	// default params
	_validParams = const_cast<Parameters*>(&_requestConfig->getParams());
	// Clean up the _requestPath, Set targetDir and targetFile based on _requestPath
	_initTargetElements();
	// if directory is not root check for location
	if (!targetDir.empty() || targetDir == "/")
	{
		_initValidLocation();
		if (_validLocation == nullptr)
			return (_setStatusCode(404));// 404 error, not found
	}
	if (!_validParams->getAllowedMethods()[_requestMethod])
		return (_setStatusCode(405));// 405 error, method not allowed
	// set indexfile if necessarry
	if (targetFile.empty() || targetFile == "/")
		targetFile = _validParams->getIndex();
	if (targetFile.empty() || targetFile == "/")
		_handleFolder();
	else
		_handleFile();
	_handleStatus();
}