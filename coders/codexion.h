/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 16:54:56 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/16 14:23:12 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

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

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	t_args			args;
}	t_dongle;

typedef struct s_coder
{
	pthread_t		thread;
	int				id;
	t_args			args;
	t_dongle		*ldongle;
	t_dongle		*rdongle;
	int				flag_burnout;
	pthread_mutex_t	*write_lock;
}	t_coder;

typedef struct s_program
{
	t_coder			*coders;
	t_dongle		*dongles;
	t_args			args;
	int				flag_stop;
	pthread_mutex_t	write_lock;
}	t_program;

/* Input validation */
t_args	*parse_input(int argc, char *argv[]);

/* Coder routine */
void	*coder_routine(void *arg);
void	print_status(t_coder *coder, char *event);

/* Utilities */
void	print_error(char *msg);
void	cleanup(t_args *args);

# endif
