/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   define.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: faru <faru@student.codam.nl>                 +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/01/23 11:27:59 by faru          #+#    #+#                 */
/*   Updated: 2024/02/08 22:56:53 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
// #define CGI_DIR					std::filesystem::path("var/www/cgi-bin")
// #define CGI_EXT_DEFAULT			std::filesystem::path(".cgi")
#define CGI_EXT_PY				std::string(".py")
#define CGI_EXT_SH				std::string(".sh")
#define BACKLOG 			10				        	// max pending connection queued up
#define MAX_TIMEOUT 		60000               		// maximum timeout with poll()
#define HEADER_MAX_SIZE 	8192						// max size of HTTP header