#include "HTTPrequest.hpp"

HTTPrequest::HTTPrequest( int socket, t_serv_list const& servers ) :
	HTTPstruct(socket, 200, HTTP_STATIC),
	_state(HTTP_REQ_HEAD_READING),
	_servers(servers),
	_contentLength(0) ,
	_contentLengthRead(0),
	_maxBodySize(-1)
{
	for (auto const& server : this->_servers)
	{
		for (auto const& address : server.getListens())
		{
			if (address.getDef() == true)
				this->_defaultServer = server;
		}
	}
	this->_handlerServer = this->_defaultServer;
}

void	HTTPrequest::parseHead( void )
{
	if (this->_state != HTTP_REQ_HEAD_READING)
		throw(RequestException({"instance in wrong state to perfom action"}, 500));

	_readHead();
	if (isDoneReadingHead())
		_validate();
}

void	HTTPrequest::parseBody( void )
{
	if (((isChunked() == false) and (isFileUpload() == false)) or
		(theresBodyToRead() == false))
		throw(RequestException({"instance in wrong state or type"}, 500));
	
	if (isChunked() == true)
		_readChunkedBody();
	else if (isFileUpload() == true)
		_readPlainBody();
	if (isDoneReadingBody())
		_setBody();
}

std::string	HTTPrequest::toString( void ) const noexcept
{
	std::string	strReq;

	strReq += getMethod();
	strReq += HTTP_DEF_SP;
	strReq += this->_url.scheme;
	strReq += "://";
	strReq += this->_url.host;
	strReq += ":";
	strReq += this->_url.port;
	strReq += this->_url.path.generic_string();
	if (!this->_url.queryRaw.empty())
	{
		strReq += "?";
		strReq += this->_url.queryRaw;
	}
	if (!this->_url.fragment.empty())
	{
		strReq += "#";
		strReq += this->_url.fragment;
	}
	strReq += HTTP_DEF_SP;
	strReq += this->_version.scheme;
	strReq += "/";
	strReq += std::to_string(this->_version.major);
	strReq += ".";
	strReq += std::to_string(this->_version.minor);
	strReq += HTTP_DEF_NL;
	if (!this->_headers.empty())
	{
		for (auto item : this->_headers)
		{
			strReq += item.first;
			strReq += ":";
			strReq += HTTP_DEF_SP;
			strReq += item.second;
			strReq += HTTP_DEF_NL;
		}
	}
	strReq += HTTP_DEF_NL;
	if (!this->_body.empty())
		strReq += this->_body;
	return (strReq);
}

std::string 	HTTPrequest::getMethod( void ) const noexcept
{
	switch (this->_method)
	{
		case (HTTP_GET):
			return ("GET");
		case (HTTP_POST):
			return ("POST");
		case (HTTP_DELETE):
			return ("DELETE");
		default:
			return ("");
	}
}

std::string		HTTPrequest::getHost( void ) const noexcept
{
	try
	{
		std::string	hostPort = this->_headers.at(HEADER_HOST);
		size_t	semiColPos = hostPort.find(':');
		return (hostPort.substr(0, semiColPos));
	}
	catch(const std::out_of_range& e) {
		return ("");
	}
}

std::string		HTTPrequest::getPort( void ) const noexcept
{
	try
	{
		std::string	hostPort = this->_headers.at(HEADER_HOST);
		size_t	semiColPos = hostPort.find(':');
		if (semiColPos == std::string::npos)
			return (HTTP_DEF_PORT);
		else
			return (hostPort.substr(semiColPos + 1));
	}
	catch(const std::out_of_range& e) {
		return ("");
	}
}

size_t	HTTPrequest::getContentLength( void ) const noexcept
{
	return (this->_contentLength);
}

std::string	const&	HTTPrequest::getQueryRaw( void ) const noexcept
{
	return (this->_url.queryRaw);
}

std::string		HTTPrequest::getContentTypeBoundary( void ) const noexcept
{
	std::string boundary = "";
	if (this->_headers.count(HEADER_CONT_TYPE) > 0)
	{
		std::string contentType = this->_headers.at(HEADER_CONT_TYPE);
		size_t delim = contentType.find('=');
		boundary = contentType.substr(delim + 1, contentType.length() - delim);
	}
	return (boundary);
}

std::string const&	HTTPrequest::getServName( void ) const noexcept
{
	return(this->_handlerServer.getPrimaryName());
}

t_path const&	HTTPrequest::getRealPath( void ) const noexcept
{

	return (this->_validator.getRealPath());
}

t_path const&	HTTPrequest::getRoot( void ) const noexcept
{
	return (this->_validator.getRoot());
}

t_path	HTTPrequest::getErrorPageFromCode( int statusCode, t_path const& defaultErrorPages )
{
	try {
		if (isDoneParsingHead() == false)	// fail occured even before validation, so no error pages, skipping directly to server ones
			throw(std::out_of_range(""));
		return (this->_validator.getErrorPages().at(statusCode));
	}
	catch(const std::out_of_range& e1) {
		try {
			return (this->_handlerServer.getParams().getErrorPages().at(statusCode));
		}
		catch(const std::out_of_range& e2) {
			try {
				this->_handlerServer = this->_defaultServer;
				return (this->_handlerServer.getParams().getErrorPages().at(statusCode));
			}
			catch(const std::out_of_range& e3) {
				for (auto const& dir_entry : std::filesystem::directory_iterator{defaultErrorPages})
				{
					if (dir_entry.path().stem() == std::to_string(statusCode))
						return (dir_entry.path());
				}
				throw(RequestException({"absolutely no HTML found for code:", std::to_string(statusCode)}, 500));
			}
		}
	}
}

bool	HTTPrequest::isRedirection( void ) const noexcept
{
	return (this->_validator.isRedirection());
}

bool	HTTPrequest::isEndConn( void ) noexcept
{
	return (this->_headers[HEADER_CONN] == "close");
}

bool	HTTPrequest::isDoneReadingHead( void ) const noexcept
{
	return (this->_state > HTTP_REQ_HEAD_READING);
}

bool	HTTPrequest::isDoneParsingHead( void ) const noexcept
{
	return (this->_state > HTTP_REQ_HEAD_PARSING);
}

bool	HTTPrequest::isDoneReadingBody( void ) const noexcept
{
	return(this->_state > HTTP_REQ_BODY_READING);
}

bool	HTTPrequest::theresBodyToRead( void ) const noexcept
{
	return (this->_state == HTTP_REQ_BODY_READING);
}

void	HTTPrequest::_readHead( void )
{
	char				buffer[HTTP_BUF_SIZE];
	ssize_t				charsRead = -1;

	if (this->_tmpHead.size() == 0)
		_resetTimeout();
	bzero(buffer, HTTP_BUF_SIZE);
	charsRead = recv(this->_socket, buffer, HTTP_BUF_SIZE, 0);
	if (charsRead < 0)
		throw(ServerException({"unavailable socket"}));
	else if (charsRead == 0)
		throw(EndConnectionException({}));
	else if (this->_tmpHead.size() + charsRead > MAX_HEADER_SIZE)
		throw(RequestException({"headers too large"}, 431));
	else if (charsRead == 0)
		_checkTimeout();
	else
	{
		this->_tmpHead += std::string(buffer, buffer + charsRead);
		if (this->_tmpHead.find(HTTP_DEF_TERM) != std::string::npos)	// look for terminator in request
			this->_state = HTTP_REQ_HEAD_PARSING;
	}
}

void	HTTPrequest::_readPlainBody( void )
{
    ssize_t 			charsRead = -1;
    char        		buffer[HTTP_BUF_SIZE];

	if (this->_contentLengthRead == 0)
	{
		_resetTimeout();
		this->_contentLengthRead += this->_tmpBody.size();
	}
	bzero(buffer, HTTP_BUF_SIZE);
	charsRead = recv(this->_socket, buffer, HTTP_BUF_SIZE, 0);
	if (charsRead < 0 )
		throw(ServerException({"unavailable socket"}));
	else if (charsRead == 0)
		_checkTimeout();
	else
	{
		if ((this->_contentLengthRead + charsRead) > this->_contentLength)
		{
			charsRead -= this->_contentLength - this->_contentLengthRead;
			this->_state = HTTP_REQ_DONE;
		}
		this->_contentLengthRead += charsRead;
		this->_tmpBody += std::string(buffer, buffer + charsRead);
	}
}

void	HTTPrequest::_readChunkedBody( void )
{
    ssize_t charsRead = -1;
	size_t	delimiter = 0;
    char	buffer[HTTP_BUF_SIZE];

	if (this->_contentLengthRead == 0)
	{
		_resetTimeout();
		this->_contentLengthRead += this->_tmpBody.size();
	}
	bzero(buffer, HTTP_BUF_SIZE);
	charsRead = recv(this->_socket, buffer, HTTP_BUF_SIZE, 0);
	if (charsRead < 0 )
		throw(ServerException({"unavailable socket"}));
	else if (charsRead == 0)
		_checkTimeout();
	else
	{
		delimiter = std::string(buffer).find(HTTP_DEF_TERM);
		if (delimiter != std::string::npos)
		{
			charsRead = delimiter + HTTP_DEF_TERM.size();
			this->_state = HTTP_REQ_DONE;
		}
		if ((this->_contentLengthRead + charsRead)> this->_maxBodySize)
			throw(RequestException({"content body is longer than the maximum allowed"}, 413));
		this->_contentLengthRead += charsRead;
		this->_tmpBody += std::string(buffer, buffer + charsRead);
	}
}

void	HTTPrequest::_validate( void )
{
	std::string strHead, strHeaders;
	size_t		endHead=0, endReq=0;

	endReq = this->_tmpHead.find(HTTP_DEF_TERM);
	endHead = this->_tmpHead.find(HTTP_DEF_NL);		// look for headers
	if (endHead >= endReq)
		throw(RequestException({"no headers"}, 400));
	strHead = this->_tmpHead.substr(0, endHead);
	strHeaders = this->_tmpHead.substr(endHead + HTTP_DEF_NL.size(), endReq + HTTP_DEF_NL.size() - endHead - 1);
	endReq += HTTP_DEF_TERM.size();
	if ((endReq + 1) < this->_tmpHead.size())		// if there's the beginning of the body
		this->_tmpBody = this->_tmpHead.substr(endReq);
	_setHead(strHead);
	_setHeaders(strHeaders);
	_checkConfig();
}

void	HTTPrequest::_checkConfig( void )
{
	this->_validator.setConfig(this->_handlerServer);
	this->_validator.setMethod(this->_method);
	this->_validator.setPath(this->_url.path);
	this->_validator.solvePath();
	this->_statusCode = this->_validator.getStatusCode();
	if (isRedirection() == true)	// (internal) redirection, look for new path
	{
		this->_method = HTTP_GET;
		if (getRealPath() != "")		// redirection 301, 302, 303, 307, and 308
			this->_url.path = getRealPath();
		else							// usually is a 40X redirect
			this->_url.path = getErrorPageFromCode(this->_statusCode, std::filesystem::path("var/www/errors"));
		_checkConfig();
	}
	if (this->_statusCode >= 400)
		throw RequestException({"validation from config file failed"}, this->_statusCode);
	_checkMaxBodySize(this->_validator.getMaxBodySize());
	_setTypeUpdateState();
}

void	HTTPrequest::_setHead( std::string const& header )
{
	std::istringstream	stream(header);
	std::string 		method, url, version;

	if (! std::getline(stream, method, HTTP_DEF_SP))
		throw(RequestException({"invalid header:", header}, 400));
	_setMethod(method);
	if (! std::getline(stream, url, HTTP_DEF_SP))
		throw(RequestException({"invalid header:", header}, 400));
	_setURL(url);
	if (! std::getline(stream, version, HTTP_DEF_SP))
		throw(RequestException({"invalid header:", header}, 400));
	_setVersion(version);
}

void	HTTPrequest::_setHeaders( std::string const& strHeaders )
{
	try {
		HTTPstruct::_setHeaders(strHeaders);
	}
	catch (const HTTPexception& e) {
		throw(RequestException(e));
	}
	// check host
	if (this->_headers[HEADER_HOST] == "")		// missing Host header
		throw(RequestException({"no Host header"}, 444));
	else if (this->_url.host == "")
		_setHostPort(this->_headers[HEADER_HOST]);
	else if (this->_headers[HEADER_HOST].find(this->_url.host) == std::string::npos)
		throw(RequestException({"hosts do not match"}, 412));
	_setHandlerServer(this->_headers[HEADER_HOST]);
	// check CL
	if (this->_headers[HEADER_CONT_LEN] == "")
	{
		if (this->_headers[HEADER_TRANS_ENCODING] == "")		// no body
			return ;
		else if (this->_headers[HEADER_TRANS_ENCODING] != "chunked")
			throw(RequestException({HEADER_TRANS_ENCODING, "required"}, 411));
	}
	else
	{
		try {
			this->_contentLength = std::stoull(this->_headers[HEADER_CONT_LEN]);
		}
		catch (const std::exception& e) {
			throw(RequestException({"invalid Content-Length"}, 400));
		}
		if (this->_tmpHead.size() > this->_contentLength)
			this->_tmpBody = this->_tmpBody.substr(0, this->_contentLength);
	}
}

void	HTTPrequest::_setHandlerServer( std::string const& hostName ) noexcept
{
	std::string tmpHostName = hostName;

	std::transform(tmpHostName.begin(), tmpHostName.end(), tmpHostName.begin(), ::tolower);
	for (auto const& server : this->_servers)
	{
		for (std::string servName : server.getNames())
		{
			std::transform(servName.begin(), servName.end(), servName.begin(), ::tolower);
			if ((servName == tmpHostName))
			{
				this->_handlerServer = server;
				return ;
			}
		}
	}
}

void	HTTPrequest::_setTypeUpdateState( void )
{
	if (this->_headers[HEADER_TRANS_ENCODING] == "chunked")
	{
		this->_type = HTTP_CHUNKED;
		if (this->_tmpBody.find(HTTP_DEF_TERM) == std::string::npos)
			this->_state = HTTP_REQ_BODY_READING;
		else
		{
			_setBody();
			this->_state = HTTP_REQ_DONE;
		}
	}
	else if (this->_headers[HEADER_CONT_TYPE].find("multipart/form-data; boundary=-") == 0)
	{
		this->_type = HTTP_FILE_UPL_CGI;
		if (this->_body.size() < this->_contentLength)
			this->_state = HTTP_REQ_BODY_READING;
		else
		{
			_setBody();
			this->_state = HTTP_REQ_DONE;
		}
	}
	else
	{
		if (this->_validator.isAutoIndex() == true)
			this->_type = HTTP_AUTOINDEX_STATIC;
		else if (this->_validator.isCGI() == true)
			this->_type = HTTP_FAST_CGI;
		this->_state = HTTP_REQ_DONE;
	}
}

void	HTTPrequest::_checkMaxBodySize( size_t maxSize )
{
	if (maxSize == 0)
		return;
	if (isChunked() == true)
	{
		if (maxSize < this->_tmpBody.size())
			throw(RequestException({"Content-Length longer than config max body length"}, 413));
	}
	else if (isFileUpload() == true)
	{
		if (maxSize < this->_contentLength)
			throw(RequestException({"Content-Length longer than config max body length"}, 413));
		else if (this->_body.size() > this->_contentLength)
			this->_tmpBody = this->_tmpBody.substr(0, this->_contentLength);
	}
	this->_maxBodySize = maxSize;
}

void    HTTPrequest::_setMethod( std::string const& strMethod )
{
	if (strMethod == "GET")
		this->_method = HTTP_GET;
	else if (strMethod == "POST")
		this->_method = HTTP_POST;
	else if (strMethod == "DELETE")
		this->_method = HTTP_DELETE;
	else if ((strMethod == "HEAD") or
			(strMethod == "PUT") or
			(strMethod == "PATCH") or
			(strMethod == "OPTIONS") or
			(strMethod == "CONNECT"))
		throw(RequestException({"unsupported HTTP method:", strMethod}, 501));
	else
		throw(RequestException({"unknown HTTP method:", strMethod}, 400));
}

void	HTTPrequest::_setURL( std::string const& strURL )
{
	size_t		delimiter;
	std::string	tmpURL=strURL;

	delimiter = tmpURL.find(":");
	if (delimiter != std::string::npos)
	{
		_setScheme(tmpURL.substr(0, delimiter));
		tmpURL = tmpURL.substr(delimiter + 1);
	}
	else
		_setScheme(HTTP_DEF_SCHEME);
	delimiter = tmpURL.find("//");
	if (delimiter != std::string::npos)
	{
		if (delimiter != 0)
			throw(RequestException({"bad format URL:", tmpURL}, 400));
		tmpURL = tmpURL.substr(2);
		delimiter = tmpURL.find("/");
		if (delimiter == 0)
			_setHostPort(LOCALHOST);
		else
			_setHostPort(tmpURL.substr(0, delimiter));
		tmpURL = tmpURL.substr(delimiter);
	}
	_setPath(tmpURL);
}

void	HTTPrequest::_setScheme( std::string const& strScheme )
{
	std::string	tmpScheme = strScheme;
	std::transform(tmpScheme.begin(), tmpScheme.end(), tmpScheme.begin(), ::toupper);
	if (tmpScheme != HTTP_DEF_SCHEME)
		throw(RequestException({"unsupported scheme:", strScheme}, 400));
	std::transform(tmpScheme.begin(), tmpScheme.end(), tmpScheme.begin(), ::tolower);
	this->_url.scheme = tmpScheme;
}

void	HTTPrequest::_setHostPort( std::string const& strURL )
{
	size_t 		delimiter = strURL.find(':');
	std::string	port;

	this->_url.host = strURL.substr(0, delimiter);
	if (delimiter != std::string::npos)	// there's the port
	{
		port = strURL.substr(delimiter + 1);
		if (port.empty())		// because    http://ABC.com:/%7esmith/home.html is still valid
		{
			this->_url.port = HTTP_DEF_PORT;
			return ;
		}
		try {
			this->_url.port = std::stoi(port);
		}
		catch(const std::exception& e ) {
			throw(RequestException({"invalid port format:", strURL.substr(delimiter + 1)}, 400));
		}
	}
	else
		this->_url.port = HTTP_DEF_PORT;
}

void	HTTPrequest::_setPath( std::string const& strPath )
{
	size_t		queryPos, fragmentPos;
	std::string	tmpPath=strPath;

	queryPos = tmpPath.find('?');
	fragmentPos = tmpPath.find('#');
	this->_url.path = tmpPath.substr(0, std::min(queryPos, fragmentPos));
	if (queryPos != std::string::npos)
	{
		tmpPath = tmpPath.substr(queryPos);
		_setQuery(tmpPath);
	}
	if (fragmentPos != std::string::npos)
	{
		tmpPath = tmpPath.substr(fragmentPos);
		_setFragment(tmpPath);
	}
}

void	HTTPrequest::_setQuery( std::string const& queries )
{
	std::string			key, value, keyValue=queries;
	size_t 				del1, del2;

	if (queries == "?")
		throw(RequestException({"empty query"}, 400));
	this->_url.queryRaw = keyValue.substr(1);
	while (true)
	{
		keyValue = keyValue.substr(1);	// remove leading '?' or '&'
		del1 = keyValue.find('=');
		if (del1 == std::string::npos)
			throw(RequestException({"invalid query:", keyValue}, 400));
		del2 = keyValue.find('&');
		key = keyValue.substr(0, del1);
		value = keyValue.substr(del1 + 1, del2 - del1 - 1);
		if (key.empty() or value.empty())
			throw(RequestException({"invalid query:", keyValue}, 400));
		this->_url.query.insert({key, value});
		if (del2 == std::string::npos)
			break;
		keyValue = keyValue.substr(del2);
	}
}

void	HTTPrequest::_setFragment( std::string const& strFragment)
{
	this->_url.fragment = strFragment.substr(1);
}

void	HTTPrequest::_setVersion( std::string const& strVersion )
{
	try {
		HTTPstruct::_setVersion(strVersion);
	}
	catch(const HTTPexception& e) {
		throw(RequestException(e));
	}
}