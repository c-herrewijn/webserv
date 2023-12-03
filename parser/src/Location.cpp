/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Location.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/03 00:06:14 by itopchu       #+#    #+#                 */
/*   Updated: 2023/12/03 00:06:14 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location(void)
{

}

Location::~Location(void)
{
	nested.clear();
}

Location::Location(const Location& copy) :
	allowedMethods(copy.allowedMethods),
	URL(copy.URL),
	params(copy.params),
	nested(copy.nested)
{

}

Location&	Location::operator=(const Location& assign)
{
	allowedMethods = assign.allowedMethods;
	URL = assign.URL;
	params = assign.params;
	nested.clear();
	nested = assign.nested;
	return (*this);
}

Location::Location(std::vector<std::string>& block, const Parameters& param)
{
	params = param;
	while (block.front() != "{")
	{
		if (block.front() == "alias")
			parseAlias(block);
		else if (block.front() == "allowMethods")
			parseAllowedMethod(block);
		else if (block.front() == "location")
		{
			Location local(block, params);
			nested.push_back(local);
		}
		else if (block.front() == "root" || block.front() == "client_max_body_size" ||
				block.front() == "autoindex" || block.front() == "index" ||
				block.front() == "error_page" || block.front() == "return")
			params.fill(block);
		else
			throw ErrorCatch("\'" + block.front() + "\" is not a valid parameter in Location context");
	}
}

void	Location::parseAllowedMethod(std::vector<std::string>& block)
{

}

void	Location::parseAlias(std::vector<std::string>& block)
{

}
