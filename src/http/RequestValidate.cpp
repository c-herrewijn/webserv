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

// t_string_map const&	RequestValidate::getErrorPages( void ) const
// {
// 	return (this->_validParams->getErrorPages());
// }

// ╔════════════════════════════════╗
// ║			SETTER PART			║
// ╚════════════════════════════════╝
// void	RequestValidate::setConfig( ConfigServer const& configServ)
// {
// 	this->_handlerServer = &configServ;
// }

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
		if (_validParams->getReturns().second == "")	// file redirect name not provided in return directive, usually an error 40X
		{
			_setStatusCode(_validParams->getReturns().first);
			return (true);
		}
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
		std::cout << "checking location with size " << this->_validParams->getMaxSize() << '\n';
		errorPage = this->_validParams->getErrorPages().at(statusCode);
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