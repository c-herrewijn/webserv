/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   define.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 18:21:34 by fra           #+#    #+#                 */
/*   Updated: 2023/12/08 03:28:41 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#define BACKLOG 100				        // max pending connection queued up
#define HEADER_MAX_SIZE 8192	        // max size of HTTP header
#define MAX_CLIENTS 30       	        // max number of clients to create for testing
#define MAX_TIMEOUT 60000               // maximum timeout with poll()
