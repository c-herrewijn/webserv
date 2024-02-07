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
# define DEF_SIZE 32
# define DEF_SIZE_TYPE 'M'

class Parameters
{
	private:
		size_t 									block_index;
		std::pair<size_t, char> 				max_size;	// Will be overwriten by last found
		bool									autoindex;	// FALSE in default, will be overwriten.
		std::unordered_set<std::string>			indexes;	// Will be searched in given order
		std::string								root;		// Last found will be used.
		std::unordered_map<size_t, std::string>	error_pages;	// Same status codes will be overwriten
		std::unordered_map<size_t, std::string>	returns;	// Same reponse codes are overwriten by the last
		void	parseRoot(std::vector<std::string>& block);
		void	parseBodySize(std::vector<std::string>& block);
		void	parseAutoindex(std::vector<std::string>& block);
		void	parseIndex(std::vector<std::string>& block);
		void	parseErrorPage(std::vector<std::string>& block);
		void	parseReturn(std::vector<std::string>& block);
		// cgi_extension
		// // upload

	public:
		void	fill(std::vector<std::string>& block);
		void	setRoot(std::string& val);
		void	setSize(long val, int c);
		void	setAutoindex(bool status);
		void	addIndex(const std::string& val);

		void setBlockIndex(size_t ref);
		const size_t& getBlockIndex(void) const;
		const std::unordered_set<std::string>& getIndexes(void) const;
		const std::pair<size_t, char>& getMaxSize(void) const;
		const std::unordered_map<size_t, std::string>& getErrorPages(void) const;
		const std::unordered_map<size_t, std::string>& getReturns(void) const;
		const bool& getAutoindex(void) const;
		const std::string& getRoot(void) const;
		/*
			To Do:
			setErrorPages
			setReturns
		*/
		Parameters(void);
		virtual ~Parameters(void);
		Parameters(const Parameters& copy);
		Parameters&	operator=(const Parameters& assign);
		class ErrorCatch : public std::exception
		{
			public:
				ErrorCatch(const std::string& message) : errorMessage(message) {}
				const char* what() const throw() override {
					return errorMessage.c_str();
				}
			private:
				std::string errorMessage;
		};
	    friend std::ostream& operator<<(std::ostream& os, const Parameters& params);
};

#endif