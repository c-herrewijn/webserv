# **************************************************************************** #
#                                                                              #
#                                                         ::::::::             #
#    Makefile                                           :+:    :+:             #
#                                                      +:+                     #
#    By: fra <fra@student.codam.nl>                   +#+                      #
#                                                    +#+                       #
#    Created: 2023/11/25 18:04:49 by fra           #+#    #+#                  #
#    Updated: 2023/11/25 19:41:13 by fra           ########   odam.nl          #
#                                                                              #
# **************************************************************************** #

SHELL := /bin/bash

NAME := webserv
SRC_DIR := src
OBJ_DIR := obj
INCLUDE := inc
MAIN := main.cpp
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


all: $(NAME)

run: all
	@clear
	@./$(NAME)

$(NAME): $(OBJ_DIR) $(OBJECTS)
	@$(CC) $(CPPFLAGS) $(IFLAGS) $(OBJECTS) $(MAIN) -o $(NAME)
	@printf "(WebServ) $(GREEN)Created program $(NAME)$(RESET)\n"

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
	@-rm -f $(NAME)
	@printf "(WebServ) $(RED)Removed executable $(NAME)$(RESET)\n"

re: fclean all

.PHONY: all, run, clean, fclean, re

.DEFAULT_GOAL:=all