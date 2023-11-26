# ╔════════════════════════════════════════════════════════════════════════╗
# ║                       Makefile for C++ exercises                       ║
# ╚════════════════════════════════════════════════════════════════════════╝
# ╭────────────────────────────────────╮
# │              CXX part              │
# ╰────────────────────────────────────╯
CPP 		:= c++
NAME		= webserver
DIR_OBJ		= ./obj
DIR_INC		= ./inc
DIR_SRC		= ./src
# **************************************************************************** #
IFLAGS		:= -I$(DIR_INC)
CPPFLAGS	= -Wall -Wextra -Werror -std=c++98
CPPFLAGS	+= -MMD -MP
# CPPFLAGS	+= -g3 #-fsanitize=address
# CPPFLAGS	+= -arch x86_64
LFLAGS		:= -L$(DIR_INC)
RM			= rm -rf
# **************************************************************************** #
SRC_EXEC	= $(DIR_SRC)/main.cpp
# **************************************************************************** #
SRCS		= $(SRC_EXEC)
OBJS		= $(SRCS:$(DIR_SRC)/%.cpp=$(DIR_OBJ)/%.o)
# **************************************************************************** #
DEPS		= $(OBJS:.o=.d)
# **************************************************************************** #
all: $(NAME)

run: all
	@clear
	@./$(NAME)

rungrid: all
	@clear
	@valgrind --tool=memcheck --leak-check=full ./$(NAME)

$(NAME): $(OBJS)
	@$(CPP) $(CPPFLAGS) $(IFLAGS) $^ -o $(NAME) $(LFLAGS)
	@echo "\r\033[K\033[32m$(NAME) \033[0mis created."

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp
	@echo -n "\033[33m$(NAME) is compiling...\033[0m\r"
	@mkdir -p $(dir $@)
	@$(CPP) $(CPPFLAGS) $(IFLAGS) -c $< -o $@ -MMD -MF $(@:.o=.d) -MT $@
	@echo -n "\e[25C$< \033[32mcompiled.\033[0m"
	@echo -n "\033[K\r"

-include $(DEPS)

clean:
	@$(RM) $(DIR_OBJ)
	@echo "Object files are removed"

fclean: clean
	@$(RM) $(NAME) $(DEPS)
	@echo "$(NAME) and dependencies are removed"

re: fclean all

.PHONY: clean all fclean re
