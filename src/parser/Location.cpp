#include "Location.hpp"

Location::Location(void)
{
	URL = DEF_URL;
	fullpath = URL;
}

Location::~Location(void)
{
	nested.clear();
}

Location::Location(const Location& copy) :
	fullpath(copy.URL),
	URL(copy.URL),
	params(copy.params),
	nested(copy.nested)
{

}

Location&	Location::operator=(const Location& assign)
{
	if (this != &assign)
	{
		fullpath = assign.fullpath;
		URL = assign.URL;
		params = assign.params;
		nested.clear();
		nested = assign.nested;
	}
	return (*this);
}

Location::Location(std::vector<std::string>& block, const Parameters& param, std::filesystem::path const& prevPath)
{
	std::vector<std::vector<std::string>> locationHolder;
	std::vector<std::string>::iterator index;
	uint64_t size = 0;
	URL = DEF_URL;
	params.inherit(param);
	block.erase(block.begin());
	if (block.front()[0] != '/')
		throw ParserException({"after 'location' expected a /URL"});
	URL = block.front();
	// it is not perfect but does the job :))
	fullpath = std::filesystem::weakly_canonical(prevPath.string() + "/");
	fullpath += std::filesystem::weakly_canonical(URL);
	fullpath = std::filesystem::weakly_canonical(fullpath);
	block.erase(block.begin());
	if (block.front() != "{")
		throw ParserException({"after '/URL' expected a '{'"});
	block.erase(block.begin());
	while (block.front() != "}" && !block.empty())
	{
		if (block.front() == "location")
		{
			index = block.begin();
			while (index != block.end() && *index != "{")
				index++;
			if (index == block.end())
				throw ParserException({"Error on location parsing"});
			index++;
			size++;
			while (size && index != block.end())
			{
				if (*index == "{")
					size++;
				else if (*index == "}")
					size--;
				index++;
			}
			if (size)
				throw ParserException({"Error on location parsing with brackets"});
			std::vector<std::string> subVector(block.begin(), index);
			block.erase(block.begin(), index);
			locationHolder.push_back(subVector);
		}
		else if (block.front() == "root" || block.front() == "client_max_body_size" ||
				block.front() == "autoindex" || block.front() == "index" ||
				block.front() == "error_page" || block.front() == "return" ||
				block.front() == "allowMethods" || block.front() == "denyMethods" ||
				block.front() == "cgi_extension" || block.front() == "cgi_allowed")
			params.fill(block);
		else
			throw ParserException({"'" + block.front() + "' is not a valid parameter in 'location' context"});
	}
	block.erase(block.begin());
	for (std::vector<std::vector<std::string>>::iterator it = locationHolder.begin(); it != locationHolder.end(); it++)
	{
		Location local(*it, params, std::filesystem::weakly_canonical(fullpath));
		nested.push_back(local);
	}
}

const std::vector<Location>& Location::getNested(void) const
{
	return (nested);
}

const Parameters&	Location::getParams(void) const
{
	return (params);
}

const std::string& Location::getURL(void) const
{
	return (URL);
}

const std::filesystem::path&	Location::getFullPath(void) const
{
	return (fullpath);
}