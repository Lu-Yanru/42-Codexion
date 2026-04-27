/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_routine.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 17:30:52 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/27 18:54:10 by yanlu            ###   ########.fr       */
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
	unsigned long	current_time;

	i = 0;
	while (i < program->args->num_coders)
	{
		current_time = get_current_time();
		pthread_mutex_lock((program->coders)[i].burnout_lock);
		if (current_time - (program->coders)[i].last_compile
				> (unsigned long) program->args->time_burnout)
		{
			print_status(&(program->coders)[i], "burned out");
			pthread_mutex_unlock((program->coders)[i].burnout_lock);
			return (1);
		}
		pthread_mutex_unlock((program->coders)[i].burnout_lock);
		i++;
	}
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
		if ((program->coders)[i].already_compiled
			>= program->args->num_compiles)
			count++;
		pthread_mutex_unlock((program->coders)[i].compiles_lock);
		i++;
	}
	if (count == program->args->num_coders)
		return (1);
	return (0);
}

/*
Broadcast to all waiting coders to recheck their queue conditions
to see if they are at the front of both dongles
or if they nned to yield (wait_for_both_front()).
*/
static void	recheck_queue(t_program *program)
{
	int	i;

	i = 0;
	while (i < program->args->num_coders)
	{
		pthread_mutex_lock(&program->dongles[i].queue_lock);
		pthread_cond_broadcast(&program->dongles[i].cond);
		pthread_mutex_unlock(&program->dongles[i].queue_lock);
		i++;
	}
}

/*
Stop all threads by setting flag_stop to 1.
*/
static void	stop_all_threads(t_program *program)
{
	pthread_mutex_lock(&program->stop_lock);
	program->flag_stop = 1;
	pthread_mutex_unlock(&program->stop_lock);
	recheck_queue(program);
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
			break ;
		if (check_compiles(program))
			break ;
		recheck_queue(program);
		usleep(1000);
	}
	stop_all_threads(program);
	return (NULL);
}
