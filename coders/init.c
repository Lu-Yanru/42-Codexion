/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 15:08:05 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/16 16:36:05 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Initalize an array of dongles.
*/
static t_dongle	*init_dongles(t_args *args)
{
	t_dongle	*dongles;
	int			i;

	dongles = malloc(sizeof(t_dongle) * args->num_coders);
	if (!dongles)
		return (NULL);
	i = 0;
	while (i < args->num_coders)
	{
		pthread_mutex_init(&(dongles[i].mutex), NULL);
		dongles[i].args = args;
		i++;
	}
	return dongles;
}

/*
Initalize an array of coders.
*/
static t_coder	*init_coders(t_args	*args, t_program *program)
{
	t_coder	*coders;
	int		i;

	coders = malloc(sizeof(t_coder) * args->num_coders);
	if (!coders)
		return (NULL);
	i = 0;
	while (i < args->num_coders)
	{
		coders[i].id = i + 1;
		coders[i].args = args;
		coders[i].ldongle = &(program->dongles[i]);
		if (i == args->num_coders - 1)
			coders[i].rdongle = &(program->dongles[0]);
		else
			coders[i].rdongle = &(program->dongles[i + 1]);
		coders[i].write_lock = &(program->write_lock);
		i++;
	}
	return coders;
}

/*
Initalize the program with an array of coders,
an array of dongles, and all of the program's global parameters.
*/
t_program	*init_program(t_args *args)
{
	t_program	*program;

	program = malloc(sizeof(t_program));
	if (!program)
		return (NULL);
	program->dongles = init_dongles(args);
	if (!program->dongles)
		return (NULL);
	program->coders = init_coders(args, program);
	if (!program->coders)
		return (NULL);
	program->args = args;
	pthread_mutex_init(&(program->write_lock), NULL);
	return (program);
}
