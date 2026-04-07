# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/11/24 11:49:58 by yanlu             #+#    #+#              #
#    Updated: 2026/04/07 18:31:42 by yanlu            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = codexion

# Source files, obj files and header files
SRC = main.c main_utils.c parse_input.c

OBJ = $(SRC:.c=.o)

IDIR = .

HEADER = codexion.h

# Compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

# Rules
all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -I$(IDIR) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
