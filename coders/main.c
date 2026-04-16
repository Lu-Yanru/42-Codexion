/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 16:50:28 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/16 16:08:06 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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
	// if (!start_program(program))
	// {
	// 	cleanup(args, program);
	// 	print_error("Fail to start program.");
	// 	return (1);
	// }
	cleanup(args, program);
}
