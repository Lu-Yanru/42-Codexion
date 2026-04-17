# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/11/24 11:49:58 by yanlu             #+#    #+#              #
#    Updated: 2026/04/17 18:05:24 by yanlu            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = codexion

SRCDIR = coders
INCDIR = coders

# Source files, obj files and header files
SRC = $(addprefix $(SRCDIR)/, \
	main.c main_utils.c parse_input.c init.c \
	coder_routine.c coder_compile.c monitor_routine.c \
)

OBJ = $(SRC:.c=.o)

HEADER = $(addprefix $(INCDIR)/, codexion.h)

# Compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -g

# Rules
all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

$(SRCDIR)/%.o: $(SRCDIR)/%.c $(HEADER)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
