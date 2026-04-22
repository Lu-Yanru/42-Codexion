/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 17:20:09 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/22 20:16:52 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

unsigned long	get_current_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

int	error_exit(char *msg, t_args *args, t_program *program)
{
	fprintf(stderr, "%s\n", msg);
	cleanup(args, program);
	return (1);
}

void	free_dongles(t_dongle *dongles, int size)
{
	int	i;

	i = 0;
	while (i < size)
	{
		pthread_mutex_destroy(&(dongles[i].mutex));
		pthread_cond_destroy(&(dongles[i].cond));
		i++;
	}
	free(dongles);
	dongles = NULL;
}

void	cleanup(t_args *args, t_program *program)
{
	int	size;

	if (args)
	{
		size = args->num_coders;
		free(args);
		args = NULL;
	}
	if (program)
	{
		pthread_mutex_destroy(&(program->write_lock));
		pthread_mutex_destroy(&(program->stop_lock));
		pthread_mutex_destroy(&(program->compiles_lock));
		pthread_mutex_destroy(&(program->burnout_lock));
		free_dongles(program->dongles, size);
		free(program->coders);
		program->coders = NULL;
		free(program);
		program = NULL;
	}
}
