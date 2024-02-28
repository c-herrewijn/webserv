/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parameters.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/02 21:03:01 by itopchu       #+#    #+#                 */
/*   Updated: 2023/12/02 21:03:01 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARAMETERS_HPP
# define PARAMETERS_HPP
# include <vector>
# include <string>
# include <unordered_set>
# include <unordered_map>
# include <climits>
# include <stdexcept>
# include <iostream>
# include <bitset>

# include "Exceptions.hpp"

# define M_GET 0
# define M_POST 1
# define M_DELETE 2
# define M_SIZE 3 // amount of methodes used in our program
# define DEF_SIZE 32
# define DEF_SIZE_TYPE 'M'
# define DEF_ROOT "/html"

class Parameters
{
	public:
		Parameters(void);
		virtual ~Parameters(void);
		Parameters(const Parameters& copy);
		Parameters&	operator=(const Parameters& assign);

		void	fill(std::vector<std::string>& block);
		void	setRoot(std::string& val);
		void	setSize(uintmax_t val, char *c);
		void	setAutoindex(bool status);

		void 											setBlockIndex(size_t ref);
		const size_t& 									getBlockIndex(void) const;
		const std::string& 								getIndex(void) const;
		std::uintmax_t									getMaxSize(void) const;
		const std::unordered_map<size_t, std::string>& 	getErrorPages(void) const;
		const std::unordered_map<size_t, std::string>& 	getReturns(void) const;
		const bool& 									getAutoindex(void) const;
		const std::string& 								getRoot(void) const;
		const std::bitset<M_SIZE>&						getAllowedMethods(void) const;

	private:
		size_t 									block_index;
		size_t					 				max_size;	// Will be overwriten by last found
		bool									autoindex;	// FALSE in default, will be overwriten.
		std::string								index;	// Will be searched in given order
		std::string								root;		// Last found will be used.
		std::unordered_map<size_t, std::string>	error_pages;	// Same status codes will be overwriten
		std::unordered_map<size_t, std::string>	returns;	// Same reponse codes are overwriten by the last
		std::bitset<M_SIZE> 					allowedMethods;	// Allowed methods

		void	_parseRoot(std::vector<std::string>& block);
		void	_parseBodySize(std::vector<std::string>& block);
		void	_parseAutoindex(std::vector<std::string>& block);
		void	_parseIndex(std::vector<std::string>& block);
		void	_parseErrorPage(std::vector<std::string>& block);
		void	_parseReturn(std::vector<std::string>& block);
		void	_parseAllowedMethod(std::vector<std::string>& block);

	    friend std::ostream& operator<<(std::ostream& os, const Parameters& params);
};

#endif