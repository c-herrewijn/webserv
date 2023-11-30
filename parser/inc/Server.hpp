/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/26 05:34:59 by itopchu       #+#    #+#                 */
/*   Updated: 2023/11/26 05:34:59 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP
# include <string>
# include <vector>

# ifndef DEF_HOST
#  define DEF_HOST "127.0.0.1"
# endif
# ifndef DEF_PORT
#  define DEF_PORT 80
# endif
# ifndef DEF_SIZE
#  define DEF_SIZE 32
# endif


//	Location parameters:
struct Location
{
	/*
		allowMethods
		alias

		Non sures:
		allow
		deny
		cgi_extension
		cgi_pass
		cgi_processing
		fastcgi_param
		fastcgi_pass
		// php_cgi_path
		// python_cgi_path
		cgi_environment_variable
	*/
	std::vector<Location>	nested;
};

// Possible duplicate members in server|location
struct Parameters
{
	std::string					max_size;	// Will be overwriten by last found
	bool						autoindex;	// FALSE in default, will be overwriten.
	std::vector<std::string>	index;		// Will be searched in given order
	std::string					root;		// Last found will be used.
	std::vector<size_t, std::string>	error_pages;	// Same status codes will be overwriten
	std::vector<size_t, std::string>	returns;	// Same reponse codes are overwriten by the last
	// cgi_extension
	// // upload
};

struct Listen
{
	int			i_ip;	// Bitwise stored ip
	int			i_port;	// Bitwise stored port
	std::string	s_ip;	// String ip
	std::string	s_port;	// String port
	bool		default;	// Check for default_server
	bool		asteriks;	// If TRUE every IP from 0 to 255 is the ip
};

struct Name
{
	bool		asterix;	// Check for any asterix in name
	std::string	name;		// Given name (not sure what to do with *)
};

class Server
{
	private:
		std::vector<Listen> listens;
		std::vector<Name>	names;
		std::string			root;
		Parameters			params;
	public:
		Server(void);
		~Server(void);
		class ErrorCatch : public std::exception {
			public:
				ErrorCatch(const char* message) : errorMessage(message) {}
				const char* what() const throw() override {
					return errorMessage.c_str();
				}
			private:
				std::string errorMessage;
		};
};

#endif