# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: itopchu <itopchu@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/11/25 18:04:49 by fra               #+#    #+#              #
#    Updated: 2023/12/12 15:53:43 by itopchu          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SHELL := /bin/bash

SERVER := webserv
CLIENT := webclient
SRC_DIR := src
OBJ_DIR := obj
INCLUDE := inc
MAIN_SERV := mainServ.cpp
MAIN_CLI := mainCli.cpp
HEADERS := $(wildcard $(INCLUDE)/*.hpp)
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)%,$(OBJ_DIR)%,$(SOURCES:.cpp=.o))	

CC  := c++
IFLAGS := -I$(INCLUDE)
CPPFLAGS = -Wall -Wextra -Werror -Wshadow -Wpedantic -g3 -fsanitize=address

GREEN = \x1b[32;01m
RED = \x1b[31;01m
BLUE = \x1b[34;01m
RESET = \x1b[0m


all: $(SERVER) $(CLIENT)

server: $(SERVER)
	@clear
	@./$(SERVER)

client: $(CLIENT)
	@clear
	@./$(CLIENT) "localhost" "4242"

$(CLIENT): $(OBJECTS) $(MAIN_CLI)
	@$(CC) $(CPPFLAGS) $(IFLAGS) $^ -o $@
	@printf "(WebServ) $(GREEN)Created program $@$(RESET)\n"

$(SERVER): $(OBJECTS) $(MAIN_SERV)
	@$(CC) $(CPPFLAGS) $(IFLAGS) $^ -o $@
	@printf "(WebServ) $(GREEN)Created program $@$(RESET)\n"

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	@$(CC) $(CPPFLAGS) $(IFLAGS) -c $< -o $@
	@printf "(WebServ) $(BLUE)Created object $$(basename $@)$(RESET)\n"

clean:
	@for file in $(OBJECTS); do \
		rm -f $$file;	\
		printf "(WebServ) $(RED)Removed object $$(basename $$file)$(RESET)\n"; \
	done

fclean: clean
	@-rm -f $(SERVER)
	@printf "(WebServ) $(RED)Removed executable $(SERVER)$(RESET)\n"
	@-rm -f $(CLIENT)
	@printf "(WebServ) $(RED)Removed executable $(CLIENT)$(RESET)\n"

re: fclean all

.PHONY: all, run, clean, fclean, re

.DEFAULT_GOAL:=all
>>>>>>> fra
