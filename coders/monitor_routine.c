/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_routine.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 17:30:52 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/17 09:49:04 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Check if any coder is burnt out,
if so, print out burnt out message
and return 1, else return 0.
*/
// static int	check_burnout(t_coder *coders)

/*
Check if all coders have compiled enough,
if so, return 1, else return 0.
*/
static int	check_compiles(t_program *program)
{
	int	i;
	int	count;

	i = 0;
	count = 0;
	while (i < program->args->num_coders)
	{
		pthread_mutex_lock((program->coders)[i].compiles_lock);
		if ((program->coders)[i].already_compiled > program->args->num_coders)
			count++;
		pthread_mutex_unlock((program->coders)[i].compiles_lock);
		i++;
	}
	if (count == program->args->num_coders)
		return (1);
	return (0);
}

/*
Stop all threads by setting flag_stop to 1.
*/
static void	stop_all_threads(t_program *program)
{
	pthread_mutex_lock(&program->stop_lock);
	program->flag_stop = 1;
	pthread_mutex_unlock(&program->stop_lock);
}

/*
Runs continously, stops when:
- One coder burnt out
- All coders compiled enough times.
Stop all coder threads and unlock all locks
when stopping condition is met.
*/
void	*monitor_routine(void *arg)
{
	t_program	*program;

	program = (t_program *)arg;
	while (1)
	{
		// if (check_burnout(program))
		// 	break;
		if (check_compiles(program))
			break;
	}
	stop_all_threads(program);
	return (NULL);
}
