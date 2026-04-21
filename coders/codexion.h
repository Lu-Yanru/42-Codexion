/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 16:54:56 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/21 18:37:13 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
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
	t_args			*args;
	unsigned long	last_used;
	unsigned long	current_ddl;
}	t_dongle;

typedef struct s_coder
{
	pthread_t		thread;
	int				id;
	t_args			*args;
	t_dongle		*ldongle;
	t_dongle		*rdongle;
	int				*flag_stop;
	int				already_compiled;
	unsigned long	start_time;
	unsigned long	last_compile;
	pthread_mutex_t	*write_lock;
	pthread_mutex_t	*stop_lock;
	pthread_mutex_t	*compiles_lock;
	pthread_mutex_t	*burnout_lock;
}	t_coder;

typedef struct s_program
{
	pthread_t		monitor;
	t_coder			*coders;
	t_dongle		*dongles;
	t_args			*args;
	unsigned long	start_time;
	int				flag_stop;
	pthread_mutex_t	write_lock;
	pthread_mutex_t	stop_lock;
	pthread_mutex_t	compiles_lock;
	pthread_mutex_t	burnout_lock;
}	t_program;

/* Input validation */
t_args	*parse_input(int argc, char *argv[]);

/* Initalization */
t_program	*init_program(t_args *args);

/* Coder routine */
void	*coder_routine(void *arg);
void	print_status(t_coder *coder, char *event);
int		compile(t_coder *coder);
int		check_stop(t_coder *coder);

/* Monitor routine */
void	*monitor_routine(void *arg);

/* Utilities */
unsigned long	get_current_time(void);
int				error_exit(char *msg, t_args *args, t_program *program);
void			cleanup(t_args *args, t_program *program);

# endif
