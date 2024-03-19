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

	_autoIndex = false;
	_isCGI = false;
	_realPath = "/IAMEMPTY";
	_requestMethod = HTTP_GET;
}

RequestValidate::~RequestValidate( void ) {}

// ╔════════════════════════════════╗
// ║			GETTER PART			║
// ╚════════════════════════════════╝
// Parameters*	RequestValidate::getValidParams( void ) const
// {
// 	return (_validParams);
// }

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

t_path	const& RequestValidate::getRoot( void ) const
{
	return (this->_validParams->getRoot());
}

t_string_map const&	RequestValidate::getErrorPages( void ) const
{
	return (this->_validParams->getErrorPages());
}

// ╔════════════════════════════════╗
// ║			SETTER PART			║
// ╚════════════════════════════════╝
void	RequestValidate::setConfig( ConfigServer const& configServ)
{
	this->_requestConfig = &configServ;
}

void	RequestValidate::setMethod( HTTPmethod method )
{
	this->_requestMethod = method;
}

void	RequestValidate::setPath( t_path const& newPath )
{
	this->_requestPath = std::filesystem::weakly_canonical(newPath);
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
Location const*	RequestValidate::_diveLocation(Location const& cur, std::vector<std::string>::iterator itDirectory, std::vector<std::string>& folders)
{
	std::vector<std::string> curURL;
	std::vector<std::string>::iterator itFolders;
	Location const*	valid;

	_separateFolders(cur.getFilesystem().string(), curURL);
	itFolders = curURL.begin();
	// std::cout << "Diving inner scope\n";
	while (itFolders != curURL.end() && itDirectory != folders.end())
	{
		if (*itFolders != *itDirectory)
			break ;
		itFolders++;
		itDirectory++;
	}
	if (itFolders == curURL.end() && itDirectory == folders.end())
	{
		// std::cout << "Matching location found\n";
		return (&cur);
	}
	else if (itFolders == curURL.end())
	{
		// std::cout << "Dives into nested\n";
		for (auto& nest : cur.getNested())
		{
			valid = _diveLocation(nest, itDirectory, folders);
			if (valid)
				return (valid);
		}
	}
	return (NULL);
}

void	RequestValidate::_initValidLocation(void)
{
	Location const*						valid;
	std::vector<std::string>			folders;
	std::vector<std::string>::iterator	it;

	_separateFolders(targetDir.string(), folders);
	// std::cout << "Prepares for diving: ";
	for (auto folder : folders)
		// std::cout << folder << " ";
	// std::cout << "\n";
	for (auto& location : _requestConfig->getLocations())
	{
		it = folders.begin();
		valid = _diveLocation(location, it, folders);
		if (valid)
			break ;
	}
	// std::cout << "End of diving\n";
	if (!valid)
	{
		// std::cout << "Not found\n";
		return ;
	}
	// std::cout << "Found in: " << valid->getURL() << "\n";
	_validLocation = valid;
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
// │     FILE/FOLDER PERMS     │
// ╰───────────────────────────╯
bool	RequestValidate::_checkPerm(t_path const& path, PermType type)
{
	t_perms perm = std::filesystem::status(path).permissions();
	switch (type)
	{
		case PERM_READ:
			return ((perm & (t_perms::owner_read | t_perms::group_read | t_perms::others_read)) != t_perms::none);
		case PERM_WRITE:
			return ((perm & (t_perms::owner_write | t_perms::group_write | t_perms::others_write)) != t_perms::none);
		case PERM_EXEC:
			return ((perm & (t_perms::owner_exec | t_perms::group_exec | t_perms::others_exec)) != t_perms::none);
		default:
			return ((perm & (t_perms::owner_all | t_perms::group_all | t_perms::others_all)) != t_perms::none);
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
	t_path dirPath = _validParams->getRoot();
	dirPath += targetDir;
	dirPath = std::filesystem::weakly_canonical(dirPath);
	_realPath = dirPath;
	_autoIndex = false;
	if (!std::filesystem::exists(dirPath) ||
	!std::filesystem::is_directory(dirPath))
		return (_setStatusCode(404), false); // 404 error, not found
	// check autoindex
	if (!_validParams->getAutoindex())
		return (_setStatusCode(404), false); // 404 error, not found
	if (!_checkPerm(dirPath, PERM_READ))
		return (_setStatusCode(403), false);
	_autoIndex = true;
	return(true);
}

bool	RequestValidate::_handleFile(void)
{
	t_path dirPath = _validParams->getRoot();
	dirPath += "/";
	dirPath += targetDir;
	dirPath += "/";
	t_path filePath = dirPath;
	filePath += "/";
	filePath += targetFile;
	_isCGI = false;
	// check filePath
	// std::cout << "Checked paths: " << dirPath << " : " << filePath << "\n";
	if (!std::filesystem::exists(filePath))
		return (_setStatusCode(404), false);
	if (std::filesystem::is_directory(filePath))
	{
		_realPath = std::filesystem::weakly_canonical(dirPath);
		return (_handleFolder());
	}

	_realPath = std::filesystem::weakly_canonical(filePath);
	// Check request type for permission check
	if (_validParams->getCgiAllowed() &&
		filePath.has_extension() &&
		filePath.extension() == _validParams->getCgiExtension())
	{
		// check permission
		if (!_checkPerm(filePath, PERM_EXEC))
			return (_setStatusCode(403), false);
		_isCGI = true;
	}
	else if (!_checkPerm(filePath, PERM_READ))
		return (_setStatusCode(403), false);
	return (true);
}

// ╭───────────────────────────╮
// │  STATUS CODE REDIRECTION  │
// ╰───────────────────────────╯
bool	RequestValidate::_handleReturns(void)
{
	if (_validParams->getReturns().first)
	{
		_statusCode = _validParams->getReturns().first;
		targetFile = std::filesystem::weakly_canonical(_validParams->getReturns().second);
		if (_handleFile())
			return (true);
		return (_handleStatus(), true);
	}
	return (false);
}

bool	RequestValidate::_handleErrorCode(void)
{
	std::map<size_t, std::string>::const_iterator it;
	it = _validParams->getErrorPages().find(_statusCode);
	if (it == _validParams->getErrorPages().end())
		return (false);
	targetFile = std::filesystem::weakly_canonical((*it).second);
	return (_handleFile());
}

bool	RequestValidate::_handleServerPages(void)
{
	// This is a temp solution. NEEDS TO BE REPLACED WITH A MACRO
	t_path lastChance = std::filesystem::current_path();
	lastChance += SERVER_DEF_PAGES;
	lastChance += (std::to_string(_statusCode) + ".html");
	// std::cout << "lastChance: " << lastChance << "\n";
	if (!std::filesystem::is_regular_file(lastChance))
		return (false);
	if (!_checkPerm(lastChance, PERM_READ))
		return (false);
	_realPath = std::filesystem::weakly_canonical(lastChance);
	return (true);
}

void	RequestValidate::_handleStatus(void)
{
	if (_statusCode < 400)
		return ;
	// handle error_pages
	if (_handleErrorCode())
		return ;
	// use server scope error pages
	_validParams = &(_requestConfig->getParams());
	if (_handleErrorCode())
		return ;
	// use servers error page
	if (_handleServerPages())
		return ;
	// final solution the case: internal server error 500
	_realPath = "/";
	targetDir.clear();
	targetFile.clear();
	_setStatusCode(500);
}

// ╭───────────────────────────╮
// │   MAIN FUNCTION TO START  │
// ╰───────────────────────────╯
void	RequestValidate::solvePath(void)
{
	// std::cout << "Enters solving\n";
	// given NULL is not valid
	if (!_requestConfig)
		throw(RequestException({"No config given. Request validation can not continue."}, 500));
	if (_requestPath.empty())
		throw(RequestException({"No Path given. Request validation can not continue."}, 500));
	// std::cout << "Requested params are set\n";
	_setStatusCode(200);
	// default params
	_validParams = &(_requestConfig->getParams());
	// Clean up the _requestPath, Set targetDir and targetFile based on _requestPath
	_initTargetElements();
	// std::cout << "Target params are set\n";
	// std::cout << targetDir << " - " << targetFile << "\n";
	// if directory is not root check for location
	// std::cout << "location check entrence\n";
	if (!targetDir.empty() || targetDir == "/")
	{
		// std::cout << "Dives into locations\n";
		_initValidLocation();
		if (_validLocation == nullptr)
		{
			// std::cout << "No location found: exit\n";
			return (_setStatusCode(404), _handleStatus());// 404 error, not found
		}
		// std::cout << "URL: " << _validLocation->getURL() << "\n";
		// std::cout << "found location filesystem: " << _validLocation->getFilesystem().c_str() << "\n";
		_validParams = &(_validLocation->getParams());
	}
	// std::cout << "Check allowed methods: " << _validParams->getAllowedMethods() << "\n";
	if (!_validParams->getAllowedMethods()[_requestMethod])
		return (_setStatusCode(405), _handleStatus());// 405 error, method not allowed
	// std::cout << "Check return param\n";
	// handle return
	if (_handleReturns())
		return ;
	// std::cout << "Update target file : " << targetFile << "\n";
	// set indexfile if necessarry
	if (targetFile.empty() || targetFile == "/")
		targetFile = _validParams->getIndex();
	// Normalization for the case user gives sth random
	targetFile = std::filesystem::weakly_canonical(targetFile);
	// std::cout << "New target file : " << targetFile << "\n";
	if (targetFile.empty() || targetFile.string() == "/")
		_handleFolder();
	else
		_handleFile();
	_handleStatus();
	// std::cout << "Realpath: " << _realPath << " : " << _statusCode << "\n";
}