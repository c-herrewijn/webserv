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

RequestValidate::RequestValidate(const ConfigServer* conf, const HTTPrequest& req)
{
	this->config = conf;
	this->request = req;
	validLocation = NULL;
	validParams = NULL;
	calculateElements();
}

RequestValidate::RequestValidate(void)
{
	request = NULL;
	config = NULL;
	validLocation = NULL;
	validParams = NULL;
}

RequestValidate::RequestValidate(const RequestValidate& copy) :
	request(copy.request),
	config(copy.config),
	validLocation(copy.validLocation),
	validParams(copy.validParams),
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
		config = assign.request;
		validLocation = assign.validLocation;
		validParams = assign.validParams;
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

const std::vector<std::string>& RequestValidate::getFolders() const
{
	return folders;
}

// Setter functions
void	RequestValidate::setRequest(HTTPrequest* req)
{
	request = req;
	calculateElements();
}

void	RequestValidate::setConfig(ConfigServer* conf)
{
	config = conf;
	calculateElements();
}

Location*	RequestValidate::diveLocation(Location& cur, std::vector<std::string>::iterator& itDirectory)
{
	std::vector<std::string> curURL;
	std::vector<std::string>::iterator itFolders;

	separateURL(cur.getURL(), curURL);
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
		setValidParams(valid->getParams());
	}
}

void	RequestValidate::setValidParams(Parameters* params)
{
	validParams = params;
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
	if (targetFile)
	{
		targetDir = request->getUrl().path.parent_path().string();
		if (targetDir.back() != '/')
			targetDir += "/";
	}
	else
		targetDir = request->getUrl().path.string();
}

void	RequestValidate::calculateElements(void)
{
	if (!request || !config)
		return ;
	setValidParams(config->getParams());
	setTargetFile(request->getUrl().path.filename());
	initTargetDir();
	if (getTargetDir() != "/")
	{
		initValidLocation();
		if (!getValidLocation())
			// 404 error, check return, error_page, default error_page, error_page creation
	}
	if (getTargetFile().empty())
		setTargetFile(getValidParams()->getIndex());
	// check Method
	// check existance depending on method (for POST existance may be a problem?)
	// check permission
	// check size
}