/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 16:50:28 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/17 09:46:55 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Start the program by creating a monitoring thread
and one thread per coder.
Return 1 if successfully started, 0 otherwise.
*/
static int	start_program(t_program *program)
{
	int			i;

	if (pthread_create(&program->monitor, NULL,
			monitor_routine, program) != 0)
		return (0);
	i = 0;
	while (i < program->args->num_coders)
	{
		if (pthread_create(&(program->coders[i].thread), NULL,
				coder_routine, &(program->coders[i])))
			return (0);
		i++;
	}
	return (1);
}

/*
Stop program, unlock all mutexes,
join all threads, free all memories.
*/
static void	stop_program(t_program *program, t_args *args)
{
	int	i;

	i = 0;
	while (i < args->num_coders)
	{
		pthread_join(program->coders[i].thread, NULL);
		i++;
	}
	pthread_join(program->monitor, NULL);
	pthread_mutex_unlock(&program->write_lock);
	pthread_mutex_unlock(&program->stop_lock);
	pthread_mutex_unlock(&program->compiles_lock);
	cleanup(args, program);
}

int	main(int argc, char *argv[])
{
	t_args		*args;
	t_program	*program;

	program = NULL;
	args = parse_input(argc, argv);
	if (!args)
	{
		print_error("Invalid input.");
		return (1);
	}
	program = init_program(args);
	if (!program)
	{
		cleanup(args, program);
		print_error("Fail to initialze program.");
		return (1);
	}
	if (!start_program(program))
	{
		cleanup(args, program);
		print_error("Fail to start program.");
		return (1);
	}
	stop_program(program, args);
}
