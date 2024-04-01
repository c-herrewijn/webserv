#include "RequestValidate.hpp"

// ╔════════════════════════════════╗
// ║		CONSTRUCTION PART		║
// ╚════════════════════════════════╝
RequestValidate::RequestValidate(t_serv_list const& servers) : _servers(servers)
{
	for (auto& server : this->_servers)
	{
		for (auto const& address : server.getListens())
		{
			if (address.getDef() == true)
				this->_defaultServer = &server;
		}
	}
	this->_handlerServer = this->_defaultServer;
	_resetValues();
}

RequestValidate::~RequestValidate( void ) {}

// ╔════════════════════════════════╗
// ║			GETTER PART			║
// ╚════════════════════════════════╝
t_path const&	RequestValidate::getRealPath( void ) const
{
	return (_realPath);
}

std::string const&	RequestValidate::getServName( void ) const
{
	return(this->_handlerServer->getPrimaryName());
}

int	RequestValidate::getStatusCode( void ) const
{
	return (_statusCode);
}

int	RequestValidate::getRedirectStatusCode( void ) const
{
	return (_redirectStatusCode);
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

bool	RequestValidate::isRedirection( void ) const
{
	return (_isRedirection);
}

t_path	const& RequestValidate::getRoot( void ) const
{
	return (this->_validParams->getRoot());
}

// ╔════════════════════════════════╗
// ║			SETTER PART			║
// ╚════════════════════════════════╝
void	RequestValidate::_resetValues( void )
{
	this->_validLocation = nullptr;
	this->_validParams = &this->_handlerServer->getParams();

	this->_autoIndex = false;
	this->_isCGI = false;
	this->_isRedirection = false;
	this->_realPath = "/IAMEMPTY";
	this->_requestMethod = HTTP_GET;
	this->_statusCode = 200;
	this->_redirectStatusCode = 200;
}

void	RequestValidate::_setMethod( HTTPmethod method )
{
	this->_requestMethod = method;
}

void	RequestValidate::_setConfig( std::string const& hostName )
{
	std::string tmpHostName = hostName;

	std::transform(tmpHostName.begin(), tmpHostName.end(), tmpHostName.begin(), ::tolower);
	for (auto& server : this->_servers)
	{
		for (std::string servName : server.getNames())
		{
			std::transform(servName.begin(), servName.end(), servName.begin(), ::tolower);
			if ((servName == tmpHostName))
			{
				this->_handlerServer = &server;
				return ;
			}
		}
	}
}

void	RequestValidate::_setPath( t_path const& newPath )
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
	// std::cout << "Prepares for diving: ";
	// for (auto folder : folders)
		// std::cout << folder << " ";
	// std::cout << "\n";
	for (auto& location : _handlerServer->getLocations())
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
			targetFile = std::filesystem::weakly_canonical(local.second);
		if (_handleFile())
			return (true);
		else
		{
			_redirectStatusCode = _validParams->getReturns().first;
			targetFile = std::filesystem::weakly_canonical(_validParams->getReturns().second);
			if (_handleFile())
			{
				_isRedirection = true;
				return (true);
			}
		}
	}
	return (false);
}

// bool	RequestValidate::_handleErrorCode(void)
// {
// 	std::map<size_t, t_path>::const_iterator it;
// 	it = _validParams->getErrorPages().find(_statusCode);
// 	if (it == _validParams->getErrorPages().end())
// 		return (false);
// 	if ((*it).second.string().front() == '/')
// 	{
// 		targetFile = std::filesystem::weakly_canonical((*it).second.filename());
// 		targetDir = std::filesystem::weakly_canonical((*it).second.parent_path());
// 	}
// 	else
// 	{
// 		targetFile = std::filesystem::weakly_canonical((*it).second);
// 	}
// 	return (_handleFile());
// }

// bool	RequestValidate::_handleServerPages(void)
// {
// 	t_path lastChance = std::filesystem::current_path();
// 	lastChance += SERVER_DEF_PAGES;
// 	lastChance += (std::to_string(_statusCode) + ".html");
// 	if (!std::filesystem::is_regular_file(lastChance))
// 		return (false);
// 	if (!_checkPerm(lastChance, PERM_READ))
// 		return (false);
// 	_realPath = std::filesystem::weakly_canonical(lastChance);
// 	return (true);
// }

// void	RequestValidate::_handleStatus(void)
// {
// 	if (_statusCode < 400)
// 		return ;
// 	if (_handleErrorCode())
// 		return ;
// 	_validParams = &(_requestConfig->getParams());
// 	if (_handleErrorCode())
// 		return ;
// 	if (_handleServerPages())
// 		return ;
// 	_realPath = "/";
// 	targetDir.clear();
// 	targetFile.clear();
// 	_setStatusCode(500);
// }

// ╭───────────────────────────╮
// │   MAIN FUNCTION TO START  │
// ╰───────────────────────────╯
void	RequestValidate::solvePath( HTTPmethod method, t_path const& path, std::string const& hostName)
{
	_resetValues();
	_setMethod(method);
	_setPath(path);
	_setConfig(hostName);
	_initTargetElements();		// Clean up the _requestPath, Set targetDir and targetFile based on _requestPath
	if (!targetDir.empty() || targetDir == "/")	// if directory is not root check for location
	{
		_initValidLocation();
		if (_validLocation == nullptr)
			return (_setStatusCode(404));
		_validParams = &(_validLocation->getParams());
	}
	if (!_validParams->getAllowedMethods()[_requestMethod])
		return (_setStatusCode(405));	// 405 error, method not allowed
	if (_handleReturns())	// handle return
		return ;
	if (targetFile.empty() || targetFile == "/")	// set indexfile if necessarry
		targetFile = _validParams->getIndex();
	targetFile = std::filesystem::weakly_canonical(targetFile);	// Normalization for the case user gives sth random
	// if (!_requestConfig)
	// 	throw(RequestException({"No config given. Request validation can not continue."}, 500));
	// if (_requestPath.empty())
	// 	throw(RequestException({"No Path given. Request validation can not continue."}, 500));
	// std::cout << "Requested path: " << _requestPath << "\n";
	// std::cout << "Request Method: " << _requestMethod << "\n";
	// _setStatusCode(200);
	// _validParams = &(_requestConfig->getParams());
	// _initTargetElements();
	// if (!targetDir.empty() || targetDir == "/")
	// {
	// 	_initValidLocation();
	// 	if (_validLocation == nullptr)
	// 		return (_setStatusCode(404), _handleStatus());
	// 	_validParams = &(_validLocation->getParams());
	// }
	// if (!_validParams->getAllowedMethods()[_requestMethod])
	// 	return (_setStatusCode(405), _handleStatus());
	// if (_handleReturns())
	// 	return ;
	// if (targetFile.empty() || targetFile == "/")
	// 	targetFile = _validParams->getIndex();
	// targetFile = std::filesystem::weakly_canonical(targetFile);
	if (targetFile.empty() || targetFile.string() == "/")
		_handleFolder();
	else
		_handleFile();
}

void	RequestValidate::solveErrorPath( int statusCode )
{
	_handleErrCode(statusCode);
	_resetValues();
	_initTargetElements();	
	if (!targetDir.empty() || targetDir == "/")	// if directory is not root check for location
	{
		_initValidLocation();
		if (_validLocation == nullptr)
			return (_setStatusCode(404));
		_validParams = &(_validLocation->getParams());
	}
	targetFile = std::filesystem::weakly_canonical(targetFile);	// Normalization for the case user gives sth random
	_handleFile();
}

void	RequestValidate::_handleErrCode( int statusCode )
{
	t_path	errorPage;
	try {
		std::cout << "code " << statusCode << " - checking location with size " << this->_validParams->getMaxSize() << '\n';
		errorPage = this->_validParams->getErrorPages().at(statusCode);
		if (this->_validLocation == nullptr)
			throw(std::out_of_range(""));
		this->_requestPath = t_path(this->_validLocation->getURL()) / std::string(errorPage).substr(1);
		std::cout << "found (not good) " << this->_requestPath << "\n";
	}
	catch(const std::out_of_range& e1) {
		try {
			std::cout << "should be going here\n";
			this->_requestPath = this->_handlerServer->getParams().getErrorPages().at(statusCode);
			std::cout << "found " << this->_requestPath << "\n";
		}
		catch(const std::out_of_range& e2) {
			try {
				this->_handlerServer = this->_defaultServer;
				this->_requestPath = this->_handlerServer->getParams().getErrorPages().at(statusCode);
			}
			catch(const std::out_of_range& e3) {
				throw(RequestException({"config doesn't provide a page for code:", std::to_string(statusCode)}, statusCode));
			}
		}
	}
}