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
void	RequestValidate::setConfig( Config const& configServ)
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
	for (auto& location : _requestConfig->getLocations())
	{
		it = folders.begin();
		valid = _diveLocation(location, it, folders);
		if (valid)
			break ;
	}
	if (!valid)
		return ;
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
		return (_setStatusCode(404), false);
	if (!_validParams->getAutoindex())
		return (_setStatusCode(404), false);
	if (!_checkPerm(dirPath, PERM_READ))
		return (_setStatusCode(403), false);
	_autoIndex = true;
	return(true);
}

bool	RequestValidate::_handleFile(void)
{
	t_path dirPath = _validParams->getRoot().string() + "/" + targetDir.string() + "/";
	t_path filePath = dirPath.string() + "/" + targetFile.string();
	_isCGI = false;
	if (!std::filesystem::exists(filePath))
		return (_setStatusCode(404), false);
	if (std::filesystem::is_directory(filePath))
	{
		_realPath = std::filesystem::weakly_canonical(dirPath);
		return (_handleFolder());
	}

	_realPath = std::filesystem::weakly_canonical(filePath);
	if (_validParams->getCgiAllowed() &&
		filePath.has_extension() &&
		filePath.extension() == _validParams->getCgiExtension())
	{
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
	auto const& local = _validParams->getReturns();
	if (local.first)
	{
		_statusCode = local.first;
		if (local.second.string().front() == '/')
		{
			targetFile = std::filesystem::weakly_canonical(local.second.filename());
			targetDir = std::filesystem::weakly_canonical(local.second.parent_path());
		}
		else
		{
			targetFile = std::filesystem::weakly_canonical(local.second);
		}
		if (_handleFile())
			return (true);
		return (_handleStatus(), true);
	}
	return (false);
}

bool	RequestValidate::_handleErrorCode(void)
{
	std::map<size_t, t_path>::const_iterator it;
	it = _validParams->getErrorPages().find(_statusCode);
	if (it == _validParams->getErrorPages().end())
		return (false);
	if ((*it).second.string().front() == '/')
	{
		targetFile = std::filesystem::weakly_canonical((*it).second.filename());
		targetDir = std::filesystem::weakly_canonical((*it).second.parent_path());
	}
	else
	{
		targetFile = std::filesystem::weakly_canonical((*it).second);
	}
	return (_handleFile());
}

bool	RequestValidate::_handleServerPages(void)
{
	t_path lastChance = std::filesystem::current_path();
	lastChance += SERVER_DEF_PAGES;
	lastChance += (std::to_string(_statusCode) + ".html");
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
	if (_handleErrorCode())
		return ;
	_validParams = &(_requestConfig->getParams());
	if (_handleErrorCode())
		return ;
	if (_handleServerPages())
		return ;
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
	if (!_requestConfig)
		throw(RequestException({"No config given. Request validation can not continue."}, 500));
	if (_requestPath.empty())
		throw(RequestException({"No Path given. Request validation can not continue."}, 500));
	_setStatusCode(200);
	_validParams = &(_requestConfig->getParams());
	_initTargetElements();
	if (!targetDir.empty() || targetDir == "/")
	{
		_initValidLocation();
		if (_validLocation == nullptr)
			return (_setStatusCode(404), _handleStatus());
		_validParams = &(_validLocation->getParams());
	}
	if (!_validParams->getAllowedMethods()[_requestMethod])
		return (_setStatusCode(405), _handleStatus());
	if (_handleReturns())
		return ;
	if (targetFile.empty() || targetFile == "/")
		targetFile = _validParams->getIndex();
	targetFile = std::filesystem::weakly_canonical(targetFile);
	if (targetFile.empty() || targetFile.string() == "/")
		_handleFolder();
	else
		_handleFile();
	_handleStatus();
}