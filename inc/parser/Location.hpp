#ifndef LOCATION_HPP
# define LOCATION_HPP
# include <string>
# include <vector>
# include <filesystem>

# include "Parameters.hpp"
# include "Exceptions.hpp"

# define DEF_URL std::string("/")

class Location
{
	public:
		Location(std::vector<std::string>& block, const Parameters& param, std::filesystem::path const&);
		virtual ~Location(void);
		Location(const Location& copy);
		Location(void);
		Location&	operator=(const Location& assign);

		const std::filesystem::path&	getFullPath(void) const;
		const std::vector<Location>&	getNested(void) const;
		const Parameters&				getParams(void) const;
		const std::string& 				getURL(void) const;

	private:
		std::filesystem::path	fullpath;
		std::string				URL; // Default "location" param
		Parameters				params; // Local values to use. If default it will be a copy of "server" context
		std::vector<Location> 	nested; // Possible nested locations
};

#endif