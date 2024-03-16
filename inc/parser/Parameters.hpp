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
# include <cstdint>
# include <stdexcept>
# include <iostream>
# include <bitset>
# include <filesystem>

# include "Exceptions.hpp"

# define M_GET 0
# define M_POST 1
# define M_DELETE 2
# define M_SIZE 3 // amount of methodes used in our program
# define DEF_SIZE 10
# define DEF_ROOT t_path("/var/www")
# define MAX_SIZE 20
# define DEF_CGI_ALLOWED false
# define DEF_CGI_EXTENTION ".cgi"

typedef std::unordered_map<size_t, std::string> t_string_map;
typedef	std::filesystem::path	t_path;

class Parameters
{
	public:
		Parameters(void);
		virtual ~Parameters(void);
		Parameters(const Parameters& copy);
		Parameters&	operator=(const Parameters& assign);

		void	fill(std::vector<std::string>& block);
		void	setRoot(t_path val);
		void	setSize(uintmax_t val, char *c);
		void	setAutoindex(bool status);

		const std::pair<size_t, std::string>& getReturns(void) const;
		void 						setBlockIndex(size_t ref);
		const size_t& 				getBlockIndex(void) const;
		const t_path&	 			getIndex(void) const;
		std::uintmax_t				getMaxSize(void) const;
		const t_string_map& 		getErrorPages(void) const;
		const bool& 				getAutoindex(void) const;
		const t_path& 				getRoot(void) const;
		const std::bitset<M_SIZE>&	getAllowedMethods(void) const;
		const std::string& 			getCgiExtension(void) const;
		const bool& 				getCgiAllowed(void) const;

	private:
		size_t 				block_index;
		std::uintmax_t		max_size;	// Will be overwriten by last found
		bool				autoindex;	// FALSE in default, will be overwriten.
		t_path				index;	// Will be searched in given order
		t_path				root;		// Last found will be used.
		t_string_map		error_pages;	// Same status codes will be overwriten
		std::pair<size_t, std::string>	returns;	// Overwritten by the last
		std::bitset<M_SIZE> allowedMethods;	// Allowed methods
		std::string			cgi_extension;	// extention .py .sh
		bool				cgi_allowed;	// Check for permissions

		void	_parseRoot(std::vector<std::string>& block);
		void	_parseBodySize(std::vector<std::string>& block);
		void	_parseAutoindex(std::vector<std::string>& block);
		void	_parseIndex(std::vector<std::string>& block);
		void	_parseErrorPage(std::vector<std::string>& block);
		void	_parseReturn(std::vector<std::string>& block);
		void	_parseAllowedMethod(std::vector<std::string>& block);
		void	_parseCgiExtension(std::vector<std::string>& block);
		void	_parseCgiAllowed(std::vector<std::string>& block);
};

#endif