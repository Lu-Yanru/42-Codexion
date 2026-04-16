/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 16:01:23 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/16 14:26:10 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Prints the status of a coder.
Uses mutex to avoid interleaved messages
from different threads.
*/
void	print_status(t_coder *coder, char *status)
{
	pthread_mutex_lock(coder->write_lock);
	printf("%d %s\n", coder->id, status);
	pthread_mutex_unlock(coder->write_lock);
}

static void	debug(t_coder *coder)
{
	print_status(coder, "is debugging");
	usleep(coder->args.time_debug * 1000);
}

static void	refactor(t_coder *coder)
{
	print_status(coder, "is refactoring");
	usleep(coder->args.time_refactor * 1000);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while(coder->flag_burnout == 0)
	{
		if (coder->args.scheduler == 0)
			compile_fifo(coder);
		else if (coder->args.scheduler == 1)
			compile_edf(coder);
		debug(coder);
		refactor(coder);
	}
	return (NULL);
}
