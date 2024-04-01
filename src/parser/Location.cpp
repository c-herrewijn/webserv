#include "Location.hpp"

Location::Location(void)
{
	block_index = 0;
	URL = DEF_URL;
	filesystem = std::filesystem::weakly_canonical(URL);
}

Location::~Location(void)
{
	nested.clear();
}

Location::Location(const Location& copy) :
	block_index(copy.block_index),
	filesystem(copy.filesystem),
	URL(copy.URL),
	params(copy.params),
	nested(copy.nested)
{

}

Location&	Location::operator=(const Location& assign)
{
	if (this != &assign)
	{
		block_index = assign.block_index;
		this->filesystem = assign.filesystem;
		URL = assign.URL;
		params = assign.params;
		nested.clear();
		nested = assign.nested;
	}
	return (*this);
}

Location::Location(std::vector<std::string>& block, const Parameters& param)
{
	std::vector<std::vector<std::string>> locationHolder;
	std::vector<std::string>::iterator index;
	uint64_t size = 0;
	URL = DEF_URL;
	filesystem = std::filesystem::weakly_canonical(URL);
	params.inherit(param);
	block.erase(block.begin());
	if (block.front()[0] != '/')
		throw ParserException({"after 'location' expected a /URL"});
	URL = block.front();
	filesystem = std::filesystem::weakly_canonical(URL);
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
				block.front() == "allowMethods" || block.front() == "cgi_extension" ||
				block.front() == "cgi_allowed")
			params.fill(block);
		else
			throw ParserException({"'" + block.front() + "' is not a valid parameter in 'location' context"});
	}
	block.erase(block.begin());
	for (std::vector<std::vector<std::string>>::iterator it = locationHolder.begin(); it != locationHolder.end(); it++)
	{
		Location local(*it, params);
		local.setBlockIndex(this->block_index);
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

const std::filesystem::path& Location::getFilesystem(void) const
{
	return (this->filesystem);
}

const std::string& Location::getURL(void) const
{
	return (URL);
}

void Location::setBlockIndex(const size_t& ref)
{
	this->block_index = ref + 1;
}

const size_t& Location::getBlockIndex(void) const
{
	return (this->block_index);
}
