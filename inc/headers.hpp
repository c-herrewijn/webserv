/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   headers.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: fra <fra@student.codam.nl.h>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/25 18:17:00 by fra           #+#    #+#                 */
/*   Updated: 2023/11/25 18:32:03 by fra           ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <unistd.h>           // execve, dup, dup2, pipe, fork, access
#include <string.h>           // strerror
#include <netdb.h>            // gai_strerror, getaddrinfo, freeaddrinfo
#include <errno.h>            // errno
#include <sys/socket.h>       // socketpair, htons, htonl, ntohs, ntohl, select, poll
// #include <sys/epoll.h>     // epoll_create, epoll_ctl, epoll_wait
// #include <sys/event.h>     // kqueue, kevent
#include <netinet/in.h>       // socket, accept, listen, bind, connect
#include <arpa/inet.h>        // htons, htonl, ntohs, ntohl
#include <sys/types.h>        // send, recv
#include <sys/socket.h>       // send, recv
#include <sys/types.h>        // chdir
#include <sys/stat.h>         // stat
#include <fcntl.h>            // open
#include <dirent.h>           // opendir, readdir, closedir
#include <sys/wait.h>         // waitpid
#include <signal.h>           // kill, signal
#include <iostream>
#include <string>
#include "colors.hpp"
#include "define.hpp"