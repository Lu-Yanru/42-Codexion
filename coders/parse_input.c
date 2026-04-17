/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_input.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 17:00:58 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/17 11:14:15 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_number(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	is_scheduler(char *str)
{
	if ((strcmp(str, "fifo") != 0) && (strcmp(str, "edf") != 0))
		return (0);
	return (1);
}

/*
A function that validates the CLI.
Returns 1 if all arguments are valid.
0 if not.
*/
static int	check_input(int argc, char *argv[])
{
	int	i;

	i = 1;
	if (argc != 9)
		return (0);
	while (i < argc - 1)
	{
		if (is_number(argv[i]) == 0)
			return (0);
		i++;
	}
	if (is_scheduler(argv[i]) == 0)
		return (0);
	return (1);
}

/*
0 for fifo, 1 for edf.
*/
static int	assign_scheduler(char *text)
{
	if (strcmp(text, "fifo") == 0)
		return (0);
	else if (strcmp(text, "edf") == 0)
		return (1);
	return (-1);
}

/*
A function that validates the CLI
and saves in the struct t_args.
Returns NULL if args are invalid.
Scheduler: 0 for fifo, 1 for edf.
*/
t_args	*parse_input(int argc, char *argv[])
{
	t_args	*args;

	if (check_input(argc, argv) == 0)
		return (NULL);
	args = malloc(sizeof(t_args));
	if (!args)
		return (NULL);
	args->num_coders = atoi(argv[1]);
	if (args->num_coders < 1)
	{
		cleanup(args, NULL);
		return (NULL);
	}
	args->time_burnout = atoi(argv[2]);
	args->time_compile = atoi(argv[3]);
	args->time_debug = atoi(argv[4]);
	args->time_refactor = atoi(argv[5]);
	args->num_compiles = atoi(argv[6]);
	args->dongle_cooldown = atoi(argv[7]);
	args->scheduler = assign_scheduler(argv[8]);
	return (args);
}
