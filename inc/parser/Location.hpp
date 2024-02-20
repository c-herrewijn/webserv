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
# include <bitset>
# include <string>
# include <vector>

# include "Parameters.hpp"
# include "Exceptions.hpp"

# define M_GET 0
# define M_POST 1
# define M_DELETE 2
# define M_SIZE 3 // amount of methodes used in our program

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
		const std::bitset<M_SIZE>&		getAllowedMethods(void) const;
		const std::string& 				getURL(void) const;

	private:
		size_t 					block_index;
		std::bitset<M_SIZE> 	allowedMethods;	// Allowed methods
		std::string				URL; // Default "location" param
		Parameters				params; // Local values to use. If default it will be a copy of "server" context
		std::vector<Location> 	nested; // Possible nested locations
	
		void	_parseAllowedMethod(std::vector<std::string>& block);
		void	_parseAlias(std::vector<std::string>& block);

    friend std::ostream& operator<<(std::ostream& os, const Location& location);
};

#endif