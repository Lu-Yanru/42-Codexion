/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 16:54:56 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/07 18:23:17 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>

typedef struct s_args
{
	int	num_coders;
	int	time_burnout;
	int	time_compile;
	int	time_debug;
	int	time_refactor;
	int	num_compiles;
	int	dongle_cooldown;
	int	scheduler;
}	t_args;

/* Input validation */
t_args	*parse_input(int argc, char *argv[]);

/* Utilities */
void	print_error(char *msg);
void	cleanup(t_args *args);

# endif
