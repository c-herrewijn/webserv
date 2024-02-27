/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Location.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/02 20:55:16 by itopchu       #+#    #+#                 */
/*   Updated: 2023/12/02 20:55:16 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP
# include <string>
# include <vector>
# include <filesystem>

# include "Parameters.hpp"
# include "Exceptions.hpp"


class Location
{
	public:
		Location(std::vector<std::string>& block, const Parameters& param);
		virtual ~Location(void);
		Location(const Location& copy);
		Location(void);
		Location&	operator=(const Location& assign);

		void 							setBlockIndex(const size_t& ref);
		const size_t& 					getBlockIndex(void) const;
		const std::vector<Location>&	getNested(void) const;
		const Parameters&				getParams(void) const;
		const std::filesystem::path&	getFilesystem(void) const;
		const std::string& 				getURL(void) const;

	private:
		size_t 					block_index;
		std::filesystem::path	filesystem; // For easier handling of URL
		std::string				URL; // Default "location" param
		Parameters				params; // Local values to use. If default it will be a copy of "server" context
		std::vector<Location> 	nested; // Possible nested locations

		void	_parseAlias(std::vector<std::string>& block);

    friend std::ostream& operator<<(std::ostream& os, const Location& location);
};

#endif