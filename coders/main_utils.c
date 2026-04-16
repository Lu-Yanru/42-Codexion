/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 17:20:09 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/16 16:39:51 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	print_error(char *msg)
{
	fprintf(stderr, "%s\n", msg);
}

void	free_dongles(t_dongle *dongles, int size)
{
	int	i;

	i = 0;
	while (i < size)
	{
		pthread_mutex_destroy(&(dongles[i].mutex));
		i++;
	}
	free(dongles);
	dongles = NULL;
}

void	cleanup(t_args *args, t_program	*program)
{
	int	size;

	size = args->num_coders;
	if (args)
	{
		free(args);
		args = NULL;
	}
	if (program)
	{
		pthread_mutex_destroy(&(program->write_lock));
		free_dongles(program->dongles, size);
		free(program->coders);
		free(program);
		program = NULL;
	}
}
