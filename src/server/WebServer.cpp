/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 17:56:25 by fra           #+#    #+#                 */
/*   Updated: 2023/12/08 04:00:01 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"
#include "CGI.hpp"

WebServer::WebServer( std::vector<ConfigServer> const& servers ) : _servers(servers)
{
	bool defServerFound = false;

	if (servers.empty() == true)
		throw(ServerException({"no Servers provided for configuration"}));
	for (auto const& server : this->_servers)
	{
		for (auto const& listAddress : server.getListens())
		{
			if (listAddress.getDef() == true)
			{
				this->_defaultServer = server;
				defServerFound = true;
			}
			auto isPresent = std::find(this->_listenAddress.begin(),this->_listenAddress.end(), listAddress);
			if ( isPresent == this->_listenAddress.end() )
				this->_listenAddress.push_back(listAddress);
		}
	}
	if (defServerFound == false)
		this->_defaultServer = servers[0];
}

WebServer::~WebServer ( void ) noexcept
{
	for (auto &item : this->_requests)
		delete item.second;
	for (auto &item : this->_responses)
		delete item.second;
	for (auto &item : this->_cgi)
		delete item.second;
	for (auto &item : this->_pollitems)
	{
		if ((item.second.pollType == LISTENER) or
			(item.second.pollType == CLIENT_CONNECTION))
			shutdown(item.first, SHUT_RDWR);
		close(item.first);
	}
}

void			WebServer::startListen( void )
{
	for (auto const& listAddress : this->_listenAddress)
	{
		try {
			this->_listenTo(listAddress.getIpString(), listAddress.getPortString());
		}
		catch(const ServerException& e) {
			std::cout << e.what() << '\n';
		}
	}
	if (this->_pollfds.empty() == true)
		throw(ServerException({"no available host:port in the configuration provided"}));
}

// NB: update for codes 20X
void			WebServer::loop( void )
{
	int					nConn = -1;

	while (true)
	{
		nConn = poll(this->_pollfds.data(), this->_pollfds.size(), 0);
		if (nConn < 0)
		{
			if ((errno != EAGAIN) and (errno != EWOULDBLOCK))
				throw(ServerException({"poll failed"}));
		}
		else if (nConn == 0)
			continue ;
		for(size_t i=0; i<this->_pollfds.size(); i++)
		{
			struct pollfd &iPollFd = this->_pollfds[i];
			t_PollItem &pollItem = this->_pollitems[iPollFd.fd];
			try {
				if (iPollFd.revents & POLLIN) {
					if (pollItem.pollState == WAITING_FOR_CONNECTION) {
						std::cerr << C_GREEN << "POLLIN - NEWCONNECTION - " << iPollFd.fd << C_RESET << std::endl;
						handleNewConnections(pollItem);
					}
					else if (pollItem.pollState == READ_REQ_HEADER) {
						std::cerr << C_GREEN << "POLLIN - READ_REQ_HEADER - " << iPollFd.fd << C_RESET << std::endl;
						readRequestHeaders(pollItem);
					}
					else if (pollItem.pollState == FORWARD_REQ_BODY_TO_CGI) {
						std::cerr << C_GREEN << "POLLIN - FORWARD_REQ_BODY_TO_CGI - " << iPollFd.fd << C_RESET << std::endl;
						readRequestBody(pollItem);
					}
					else if (pollItem.pollState == READ_CGI_RESPONSE) {
						std::cerr << C_GREEN << "POLLIN - READ_CGI_RESPONSE " << iPollFd.fd << C_RESET << std::endl;
						readCGIResponses(pollItem);
					}
					else if (pollItem.pollState == READ_STATIC_FILE) {
						std::cerr << C_GREEN << "POLLIN - READ_STATIC_FILE - " << iPollFd.fd << C_RESET << std::endl;
						readStaticFiles(pollItem);
					}
				}
				else if (iPollFd.revents & POLLOUT)
				{
					if (pollItem.pollState == FORWARD_REQ_BODY_TO_CGI) {
						std::cerr << C_GREEN << "POLLOUT - FORWARD_REQ_BODY_TO_CGI - " << iPollFd.fd << C_RESET << std::endl;
						writeToCGI(pollItem);
					}
					else if (pollItem.pollState == WRITE_TO_CLIENT) {
						std::cerr << C_GREEN << "POLLOUT - WRITE_TO_CLIENT - " << iPollFd.fd << C_RESET << std::endl;
						writeToClients(pollItem);
					}
				}
				else if (iPollFd.revents & (POLLHUP | POLLERR | POLLNVAL))	// client-end side was closed / error / socket not valid
					this->_emptyConns.push_back(iPollFd.fd);
			}
			catch (const ServerException& e) {
				std::cerr << e.what() << '\n';
				// error handling
				this->_emptyConns.push_back(iPollFd.fd);
			}
			catch (const HTTPexception& e) {
				// std::cerr << e.what() << '\n';
				redirectToErrorPage(pollItem, e.getStatus());
			}
		}
		_clearEmptyConns();
	}
}

std::string		WebServer::getAddress( const struct sockaddr_storage *addr ) const noexcept
{
	std::string ipAddress;
	if (addr->ss_family == AF_INET)
	{
		char ipv4[INET_ADDRSTRLEN];
		struct sockaddr_in *addr_v4 = (struct sockaddr_in*) addr;
		inet_ntop(addr_v4->sin_family, &(addr_v4->sin_addr), ipv4, sizeof(ipv4));
		ipAddress = std::string(ipv4) + std::string(":") + std::to_string(ntohs(addr_v4->sin_port));
	}
	else if (addr->ss_family == AF_INET6)
	{
		char ipv6[INET6_ADDRSTRLEN];
		struct sockaddr_in6 *addr_v6 = (struct sockaddr_in6*) addr;
		inet_ntop(addr_v6->sin6_family, &(addr_v6->sin6_addr), ipv6, sizeof(ipv6));
		ipAddress = std::string(ipv6) + std::string(":") + std::to_string(ntohs(addr_v6->sin6_port));
	}
	return (ipAddress);
}

ConfigServer const&	WebServer::getHandler( std::string const& servName ) const noexcept
{
	std::string	tmpServName = servName;

	for (auto const& server : this->_servers)
	{
		for (auto name : server.getNames())
		{
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			std::transform(tmpServName.begin(), tmpServName.end(), tmpServName.begin(), ::tolower);
			if (tmpServName == name)
				return (server);
		}
	}
	return (getDefaultServer());
}

ConfigServer const&	WebServer::getDefaultServer( void ) const noexcept
{
	return (this->_defaultServer);
}

void	WebServer::_listenTo( std::string const& hostname, std::string const& port )
{
	struct addrinfo *tmp, *list, filter;
	struct sockaddr_storage	hostIP;
	int yes=1, listenSocket=-1;

	bzero(&filter, sizeof(struct addrinfo));
	filter.ai_flags = AI_PASSIVE;
	filter.ai_family = AF_UNSPEC;
	filter.ai_protocol = IPPROTO_TCP;
	if (getaddrinfo(hostname.c_str(), port.c_str(), &filter, &list) != 0)
		throw(ServerException({"failed to get addresses for", hostname, ":", port}));
	for (tmp=list; tmp!=nullptr; tmp=tmp->ai_next)
	{
		listenSocket = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
		if (listenSocket == -1)
			continue;
		fcntl(listenSocket, F_SETFL, O_NONBLOCK);
		if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0)
			std::cout << "failed to update socket, trying to bind anyway... \n";
		if (bind(listenSocket, tmp->ai_addr, tmp->ai_addrlen) == 0)
			break;
		close(listenSocket);
	}
	if (tmp == nullptr)
	{
		freeaddrinfo(list);
		std::cout << "no available IP host found for " << hostname << " port " << port << '\n';
		return ;
	}
	memmove(&hostIP, tmp->ai_addr, std::min(sizeof(struct sockaddr), sizeof(struct sockaddr_storage)));
	freeaddrinfo(list);
	if (listen(listenSocket, BACKLOG) != 0)
	{
		close(listenSocket);
		throw(ServerException({"failed listen on", this->getAddress(&hostIP)}));
	}
	std::cout << "listening on: " << this->getAddress(&hostIP) << "\n";
	this->_addConn(listenSocket, LISTENER, WAITING_FOR_CONNECTION);
}

void	WebServer::_addConn( int newSocket , fdType typePollItem, fdState statePollItem )
{
	if (newSocket == -1)
		throw(ServerException({"invalid file descriptor"}));
	this->_pollfds.push_back({newSocket, POLLIN | POLLOUT, 0});
	t_PollItem newPollitem = {newSocket, typePollItem, statePollItem, false};
	this->_pollitems.insert(std::pair(newSocket, newPollitem));
}

void	WebServer::_dropConn(int toDrop) noexcept
{
	shutdown(toDrop, SHUT_RDWR);
	close(toDrop);
	for (auto curr=this->_pollfds.begin(); curr != this->_pollfds.end(); curr++)
	{
		if (curr->fd == toDrop)
		{
			this->_pollfds.erase(curr);
			break;
		}
	}
	if (this->_pollitems[toDrop].pollType == CLIENT_CONNECTION)
	{
		if (this->_requests[toDrop])
		{
			delete this->_requests[toDrop];
			this->_requests.erase(toDrop);
		}
		if (this->_responses[toDrop])
		{
			delete this->_responses[toDrop];
			this->_responses.erase(toDrop);
		}
		if (this->_cgi[toDrop])
		{
			delete this->_cgi[toDrop];
			this->_cgi.erase(toDrop);
		}
	}
	this->_pollitems.erase(toDrop);

}

void	WebServer::_clearEmptyConns( void ) noexcept
{
	while (this->_emptyConns.empty() == false)
	{
		_dropConn(this->_emptyConns.back());
		this->_emptyConns.pop_back();
	}

}

std::string	WebServer::_getHTMLfromCode( int code ) const noexcept
{
	std::filesystem::path	HTMLfolder = HTML_ERROR_FOLDER;
	std::string				filePath = DEFAULT_ERROR_PAGE_PATH.c_str();

	for (auto const& dir_entry : std::filesystem::directory_iterator{HTMLfolder})
	{
		if (dir_entry.path().stem() == std::to_string(code))
			filePath = dir_entry.path().c_str();
	}
	return (filePath);
}

int		WebServer::_getSocketFromPollitem( t_PollItem const& pollItem ) noexcept
{
	if (pollItem.pollType == CGI_DATA_PIPE)
	{
		for (auto& item : this->_cgi)
		{
			if (*item.second->getuploadPipe() == pollItem.fd)
				return (item.second->getRequestSocket());
		}
	}
	else if (pollItem.pollType == CGI_RESPONSE_PIPE)
	{
		for (auto& item : this->_cgi)
		{
			if (*item.second->getResponsePipe() == pollItem.fd)
				return (item.second->getRequestSocket());
		}
	}
	else if (pollItem.pollType == STATIC_FILE)
	{
		for (auto& item : this->_responses)
		{
			if (item.second->getHTMLfd() == pollItem.fd)
				return (item.first);
		}
	}
	else
		return (pollItem.fd);
	return (-1);	// entity not found, this should never happen
}

void	WebServer::handleNewConnections( t_PollItem& item )
{
	struct sockaddr_storage client;
	unsigned int 			sizeAddr = sizeof(client);
	int 					connfd = -1;

	connfd = accept(item.fd, (struct sockaddr *) &client, &sizeAddr);
	if (connfd == -1)
		throw(ServerException({"connection with", this->getAddress(&client), "failed"}));
	fcntl(connfd, F_SETFL, O_NONBLOCK);
	std::cout << "connected to " << this->getAddress(&client) << '\n';
	this->_addConn(connfd, CLIENT_CONNECTION, READ_REQ_HEADER);
}

void	WebServer::readRequestHeaders( t_PollItem& pollItem )
{
	HTTPrequest 	*request = nullptr;
	HTTPresponse	*response = nullptr;
	CGI 			*cgiPtr = nullptr;
	int 			HTMLfd = -1;

	request = new HTTPrequest(pollItem.fd);
	response = new HTTPresponse(pollItem.fd);
	this->_requests.insert(std::pair<int, HTTPrequest*>(pollItem.fd, request));
	this->_responses.insert(std::pair<int, HTTPresponse*>(pollItem.fd, response));
	
	response->setServName(this->getDefaultServer().getPrimaryName());
	
	request->parseHead();
	
	request->setConfigServer(&this->getHandler(request->getHost()));	// NB: needs to change
	response->setServName(request->getConfigServer().getPrimaryName());
	
	// validation from configServer (chocko's validation)
	// variable HTTPrequest.realPath must be updated!
	
	request->checkHeaders(1000000);	// has to be dynamic
	// request->setExecPath(request->getPath());	// realPath should be givien from validation


	if (request->isCGI()) {
		cgiPtr = new CGI(*request);
		this->_cgi.insert(std::pair<int, CGI*>(pollItem.fd, cgiPtr));
		this->_addConn(cgiPtr->getResponsePipe()[0], CGI_RESPONSE_PIPE, READ_CGI_RESPONSE);
		cgiPtr->run();
		if (request->hasBody()) {
			this->_addConn(cgiPtr->getuploadPipe()[1], CGI_DATA_PIPE, FORWARD_REQ_BODY_TO_CGI);
			pollItem.pollState = FORWARD_REQ_BODY_TO_CGI;
		}
		else {
			pollItem.pollState = READ_CGI_RESPONSE;
		}
	}
	else
	{
		// // std::cout << "1" << "\n";
		// if (request->getExecPath() == "/")		// NB: should be done by validation
		// 	request->setExecPath(MAIN_PAGE_PATH);
		// else if (request->getExecPath().extension() == ".ico")	// NB: should be done by validation
		// {
		// // std::cout << "2" << "\n";
		// 	response->updateContentType(ICO_CONTENT_TYPE);	// NB: should be done by validation
		// 	request->setExecPath(FAVICON_PATH);
		// }
		// else if ((request->getExecPath().filename() == "main.css"))	// NB: should be done by validation
		// 	request->setExecPath(t_path("var/www/main.css"));
		// else if (! std::filesystem::exists(request->getExecPath()))		// NB: should be done by validation
		// {
		// // std::cout << "3" << "\n";	
		// 	// std::cout << request->getExecPath() << '\n';
		// 	throw(RequestException({"path not found"}, 404));
		// }

		// // std::cout << "4" << "\n";

		// is path a file:
		//	yes:
		//	no: reading ls

		HTMLfd = open(request->getExecPath().c_str(), O_RDONLY);
		response->setHTMLfd(HTMLfd);
		_addConn(HTMLfd, STATIC_FILE, READ_STATIC_FILE);
		pollItem.pollState = READ_STATIC_FILE;
	}
}

void	WebServer::readStaticFiles( t_PollItem& currentPoll )
{
	int 			socket = _getSocketFromPollitem(currentPoll);
	HTTPresponse	*response = nullptr;

	if (socket == -1)		// this should not happen
		throw(ServerException({"response not found"}));
	response = this->_responses.at(socket);
	response->readHTML();
	if (response->isDoneReadingHTML())
	{
		this->_emptyConns.push_back(currentPoll.fd);
		this->_pollitems[response->getSocket()].pollState = WRITE_TO_CLIENT;
	}
}

// what is the triggering poll item?
void	WebServer::readRequestBody( t_PollItem& socket )
{
	std::cout << "readRequestBody - socketFd: " << socket.fd <<std::endl; // debug
	// TODO: only needs to be done when (request->_tmpBody == "")
}

void	WebServer::writeToCGI( t_PollItem& pollItem)
{
	std::cout << "cgiUploadPipeFd - socketFd: " << pollItem.fd <<std::endl; // debug
	HTTPrequest *request = nullptr;
	for (auto& cgiMapItem : this->_cgi) {
		if (cgiMapItem.second->getuploadPipe()[1] == pollItem.fd) {
			request = this->_requests[cgiMapItem.first];
			break;
		}
	}
	if (request != nullptr) {
		close(this->_cgi[request->getSocket()]->getuploadPipe()[0]);
		std::string tmpBody = request->getTmpBody();
		if (tmpBody != "") {
			write(pollItem.fd, tmpBody.data(), tmpBody.length());
			request->setTmpBody("");

			// drop from pollList after writing is done
			close(this->_cgi[request->getSocket()]->getuploadPipe()[1]);
			// NB.: add to the emptyCon list 
		}
	}
}

void	WebServer::readCGIResponses( t_PollItem& pollItem )
{
	int 			socket = _getSocketFromPollitem(pollItem);
	CGI				*cgi = nullptr;
	HTTPresponse	*response = nullptr;

	if (socket == -1)	// that should never happen
		throw(ServerException({"cgi not found"}));
	cgi = this->_cgi.at(socket);
	ssize_t	readChars = -1;
	char 	buffer[DEF_BUF_SIZE];
	bzero(buffer, DEF_BUF_SIZE);
	readChars = read(pollItem.fd, buffer, DEF_BUF_SIZE);
	if (readChars < 0)
		throw(CGIexception({"cgi pipe not available"}, 500));
	cgi->appendResponse(std::string(buffer, buffer + readChars));

	// TODO: support partial reads, i.e. in case of very big CGI response
	// a logic to understand when the whole CGI response is received is necessary
	// to know when to close the pipe connection
	if (true) // TODO keep pipe open for consequtive reads if needed
	{
		response = this->_responses[socket];
		response->parseFromCGI(cgi->getResponse());
		this->_emptyConns.push_back(pollItem.fd);
		this->_pollitems[socket].pollState = WRITE_TO_CLIENT;
	}
}

void	WebServer::writeToClients( t_PollItem& pollItem )
{
	HTTPrequest 	*request = this->_requests[pollItem.fd];
	HTTPresponse 	*response = this->_responses[pollItem.fd];

	if (!request or !response)	// that should never happen
		throw(ServerException({"cgi not found"}));
	if (request->isCGI()) {
		CGI *cgi = this->_cgi[pollItem.fd];
		response->parseFromCGI(cgi->getResponse());
		delete (cgi);
		this->_cgi.erase(pollItem.fd);
	}
	else
		response->parseFromStatic();
	response->writeContent();
	if (response->isDoneWriting())
	{
		if (request->isEndConn())
			this->_emptyConns.push_back(pollItem.fd);
		else
			this->_pollitems[pollItem.fd].pollState = READ_REQ_HEADER;
		delete (response);
		this->_responses.erase(pollItem.fd);
		delete (request);
		this->_requests.erase(pollItem.fd);
	}
}

void	WebServer::redirectToErrorPage( t_PollItem& pollItem, int statusCode ) noexcept
{
	HTTPresponse	*response = nullptr;
	HTTPrequest		*request = nullptr;
	int 			socket = _getSocketFromPollitem(pollItem);
	std::string		HTMLpath;
	int				HTMLfd = -1;

	if (socket == -1)	// that should never happen
	{
		this->_emptyConns.push_back(pollItem.fd);
		return ;
	}
	request = this->_requests.at(socket);
	response = this->_responses.at(socket);
	response->setStatusCode(statusCode);
	if (pollItem.pollType == STATIC_FILE)
		this->_emptyConns.push_back(pollItem.fd);
	else if ((pollItem.pollType == CGI_DATA_PIPE) or
		(pollItem.pollType == CGI_RESPONSE_PIPE))
	{
		this->_emptyConns.push_back(pollItem.fd);
		if (this->_cgi[socket])
		{
			delete (this->_cgi[socket]);
			this->_cgi.erase(socket);
		}
	}
	HTMLpath = _getHTMLfromCode(response->getStatusCode());
	request->setExecPath(HTMLpath);
	if (statusCode != 500)
	{
		HTMLfd = open(HTMLpath.c_str(), O_RDONLY);
		response->setHTMLfd(HTMLfd);
		_addConn(HTMLfd, STATIC_FILE, READ_STATIC_FILE);
		this->_pollitems[socket].pollState = READ_STATIC_FILE;
	}
	else
		this->_pollitems[socket].pollState = WRITE_TO_CLIENT;
}
