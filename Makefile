# **************************************************************************** #
#                                                                              #
#                                                         ::::::::             #
#    Makefile                                           :+:    :+:             #
#                                                      +:+                     #
#    By: itopchu <itopchu@student.42.fr>              +#+                      #
#                                                    +#+                       #
#    Created: 2023/11/25 18:04:49 by fra           #+#    #+#                  #
#    Updated: 2024/02/11 03:11:48 by fra           ########   odam.nl          #
#                                                                              #
# **************************************************************************** #

SHELL := /bin/bash

NAME := webserv
SRC_DIR := src
OBJ_DIR := obj
INCLUDE := inc
HEADERS := $(shell find $(INCLUDE) -type f -name '*.hpp')
SOURCES := $(shell find $(SRC_DIR) -type f -name '*.cpp')
OBJECTS := $(patsubst $(SRC_DIR)%,$(OBJ_DIR)%,$(SOURCES:.cpp=.o))

CLI_DIR := _client
CLIENT := $(CLI_DIR)/webclient
CLIENT_SRCS := $(shell find $(CLI_DIR) -type f -name '*.cpp')

CC  := c++
IFLAGS := -I$(INCLUDE) -I$(INCLUDE)/http -I$(INCLUDE)/parser -I$(INCLUDE)/server
CPPFLAGS = -Wall -Wextra -Werror -Wshadow -Wpedantic -g3 -fsanitize=address -std=c++17 

GREEN = \x1b[32;01m
RED = \x1b[31;01m
BLUE = \x1b[34;01m
RESET = \x1b[0m

all: $(NAME)

run: $(NAME)
	clear
	@./$(NAME)

$(NAME): $(OBJECTS)
	@$(CC) $(CPPFLAGS) $(IFLAGS) $^ -o $@
	@printf "(WebServ) $(GREEN)Created program $@$(RESET)\n"
	
$(OBJ_DIR):
	echo $(SOURCES)
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) Makefile
	@mkdir -p $(dir $@)
	@$(CC) $(CPPFLAGS) $(IFLAGS) -c $< -o $@
	@printf "(WebServ) $(BLUE)Created object $$(basename $@)$(RESET)\n"

client: $(CLIENT)
	@clear
	@./$(CLIENT) "localhost" "8000"

$(CLIENT): $(CLIENT_SRCS)
	@$(CC) $(CPPFLAGS) $^ -o $@
	@printf "(WebServ) $(GREEN)Created program $@$(RESET)\n"

clean:
	@for file in $(OBJECTS); do \
		rm -f $$file;	\
		printf "(WebServ) $(RED)Removed object $$(basename $$file)$(RESET)\n"; \
	done

fclean: clean
	@-rm -f $(NAME)
	@printf "(WebServ) $(RED)Removed executable $(NAME)$(RESET)\n"
	@-rm -f $(CLIENT)
	@printf "(WebServ) $(RED)Removed executable $(CLIENT)$(RESET)\n"

re: fclean all

.PHONY: all, run, client, clean, fclean, re

.DEFAULT_GOAL:=all
