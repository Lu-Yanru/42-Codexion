/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 16:01:23 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/17 17:58:37 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Check the stop flag with lock.
*/
int	check_stop(t_coder *coder)
{
	pthread_mutex_lock(coder->stop_lock);
	if (*coder->flag_stop != 0)
	{
		pthread_mutex_unlock(coder->stop_lock);
		return (1);
	}
	pthread_mutex_unlock(coder->stop_lock);
	return (0);
}

/*
Prints the status of a coder.
Uses mutex to avoid interleaved messages
from different threads.
*/
void	print_status(t_coder *coder, char *status)
{
	unsigned long	passed_time;

	passed_time = get_current_time() - coder->start_time;
	pthread_mutex_lock(coder->write_lock);
	printf("%lu %d %s\n", passed_time, coder->id, status);
	pthread_mutex_unlock(coder->write_lock);
}

static int	debug(t_coder *coder)
{
	if (!check_stop(coder))
	{
		print_status(coder, "is debugging");
		usleep(coder->args->time_debug * 1000);
		return (1);
	}
	return (0);
}

static int	refactor(t_coder *coder)
{
	if (!check_stop(coder))
	{
		print_status(coder, "is refactoring");
		usleep(coder->args->time_refactor * 1000);
		return (1);
	}
	return (0);
}

/*
Coders compile -> debug -> refactor
while checking if monitor tells them to stop.
*/
void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while(!check_stop(coder))
	{
		if (!compile(coder))
			break ;
		// if (coder->args->scheduler == 0)
		// 	compile_fifo(coder);
		// else if (coder->args->scheduler == 1)
		// 	compile_edf(coder);
		if (!debug(coder))
			break ;
		if (!refactor(coder))
			break ;
	}
	return (NULL);
}
