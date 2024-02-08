/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Listen.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: itopchu <itopchu@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/12/03 01:47:41 by itopchu       #+#    #+#                 */
/*   Updated: 2023/12/03 01:47:41 by itopchu       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LISTEN_HPP
# define LISTEN_HPP
# include <string>
# include <vector>
#include <cstdint>
# define DEF_PORT "80"
# define DEF_HOST "127.0.0.1"
# define MAX_PORT 65535
class Listen
{
	private:
		uint16_t				i_port;	// Port val;
		std::vector<uint8_t>	i_ip;	// Ip vals
		std::string				s_ip;	// String ip
		std::string				s_port;	// String port
		bool					def;	// Check for default_server
		bool					all;	// Check for *:"port"

		void	fillFull(std::vector<std::string>& block);
		void	fillIp(std::vector<std::string>& block);
		void	fillPort(std::vector<std::string>& block);

	public:
		void						fillValues(std::vector<std::string>& block);
		void						setDef(bool	status);
		void						setAll(bool status);
		const std::string&			getIpString(void) const;
		const std::vector<uint8_t>&	getIpInt(void) const;
		const std::string&			getPortString(void) const;
		const uint16_t&				getPortInt(void) const;
		const bool&					getDef(void) const;
		const bool&					getAll(void) const;
		bool						operator==(const Listen&) const;
		bool						operator!=(const Listen&) const;

		Listen(const Listen& copy);
		Listen&	operator=(const Listen& assign);
		Listen(void);
		virtual ~Listen(void);
		class ErrorCatch : public std::exception {
			public:
				ErrorCatch(const std::string& message) : errorMessage(message) {}
				const char* what() const throw() override {
					return errorMessage.c_str();
				}
			private:
				std::string errorMessage;
		};
		friend std::ostream& operator<<(std::ostream& os, const Listen& listen);
};

#endif