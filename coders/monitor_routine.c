/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_routine.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 17:30:52 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/17 11:36:42 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Check if any coder is burnt out,
if so, print out burnt out message
and return 1, else return 0.
*/
static int	check_burnout(t_program *program)
{
	int	i;

	i = 0;
	while (i < program->args->num_coders)
	{
		unsigned long	current_time;

		current_time = get_current_time();
		pthread_mutex_lock((program->coders)[i].burnout_lock);
		if (current_time - (program->coders)[i].last_compile
			> (unsigned long) program->args->time_burnout
			&& (program->coders)[i].flag_is_compiling == 0)
		{
			print_status(&(program->coders)[i], "burned out");
			pthread_mutex_unlock((program->coders)[i].burnout_lock);
			return (1);
		}
		i++;
	}
	pthread_mutex_unlock((program->coders)[i].burnout_lock);
	return (0);
}

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
		if (check_burnout(program))
			break;
		if (check_compiles(program))
			break;
	}
	stop_all_threads(program);
	return (NULL);
}
