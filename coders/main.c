/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 16:50:28 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/21 18:36:53 by yanlu            ###   ########.fr       */
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

	i = 0;
	while (i < program->args->num_coders)
	{
		if (pthread_create(&(program->coders[i].thread), NULL,
				&coder_routine, &(program->coders[i])) != 0)
			return (0);
		i++;
	}
	if (pthread_create(&program->monitor, NULL,
			&monitor_routine, program) != 0)
		return (0);
	return (1);
}

/*
Stop program, unlock all mutexes,
join all threads, free all memories.
*/
static int	stop_program(t_program *program, t_args *args)
{
	int	i;

	i = 0;
	while (i < args->num_coders)
	{
		if (pthread_join(program->coders[i].thread, NULL) != 0)
			return (0);
		i++;
	}
	if (pthread_join(program->monitor, NULL) != 0)
		return (0);
	cleanup(args, program);
	return (1);
}

int	main(int argc, char *argv[])
{
	t_args		*args;
	t_program	*program;

	program = NULL;
	args = parse_input(argc, argv);
	if (!args)
		return (error_exit("Invalid input.", NULL, NULL));
	if (args->num_compiles == 0)
		return (0);
	program = init_program(args);
	if (!program)
		return (error_exit("Fail to initialze program.", args, program));
	if (!start_program(program))
		return (error_exit("Fail to start program.", args, program));
	if (!stop_program(program, args))
		return (error_exit("Fail to stop program.", args, program));
}
