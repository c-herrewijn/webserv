/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   define.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/23 11:27:59 by faru          #+#    #+#                 */
/*   Updated: 2024/01/23 11:28:47 by faru          ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#define HTTP_DEF_PORT		80			// default port	- 				NB has to be set from config file!
#define HTTP_SCHEME			std::string("HTTP")			
#define HTTPS_SCHEME		std::string("HTTPS")
#define HTTP_TERM			std::string("\r\n\r\n")		// http terminator
#define HTTP_NL				std::string("\r\n")			// http delimiter
#define HTTP_SP				std::string(" ")			// shortcut for space
// #define CGI_DIR					std::filesystem::path("var/www/cgi-bin")
// #define CGI_EXT_DEFAULT			std::filesystem::path(".cgi")
#define CGI_EXT_PY				std::string(".py")
#define CGI_EXT_SH				std::string(".sh")
#define BACKLOG 			10				        	// max pending connection queued up
#define MAX_TIMEOUT 		60000               		// maximum timeout with poll()
#define HEADER_MAX_SIZE 	8192						// max size of HTTP header - 	NB has to be set from config file!