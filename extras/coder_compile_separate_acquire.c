/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_compile.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 12:25:08 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/22 14:58:35 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Set the priority queue based on scheduler.
*/
static void	set_priority(t_coder *coder, t_dongle *dongle)
{
	t_queue_node	my_node;

	pthread_mutex_lock(&dongle->mutex);
	my_node.coder_id = coder->id;
	if (coder->args->scheduler == 0)
		my_node.priority = dongle->ticket++;
	else
		my_node.priority = coder->last_compile + coder->args->time_burnout;
	enqueue(&dongle->queue, my_node);
	while (dongle->queue.queue[0].coder_id != coder->id)
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
}

/*
Lock the dongle if the cooldown time has passed
and if the stop flag is 0,
and if the coder ticket is at the front of the queue.
Return 1 if successfully lock the dongle, 0 otherwise.
*/
static int	lock_dongle(t_coder *coder, t_dongle *dongle)
{
	set_priority(coder, dongle);
	while (get_current_time()
			< dongle->last_used + coder->args->dongle_cooldown)
	{
		pthread_mutex_unlock(&dongle->mutex);
		if (check_stop(coder))
		{
			pthread_mutex_lock(&dongle->mutex);
			dequeue_and_wake(dongle);
			pthread_mutex_unlock(&dongle->mutex);
			return (0);
		}
		usleep(100);
		pthread_mutex_lock(&dongle->mutex);
	}
	if (check_stop(coder))
	{
		dequeue_and_wake(dongle);
		pthread_mutex_unlock(&dongle->mutex);
		return (0);
	}
	print_status(coder, "has taken a dongle");
	return (1);
}

/*
Record the time when the dongle was used
and unlock the dongle.
*/
static void	unlock_dongle(t_dongle *dongle)
{
	dongle->last_used = get_current_time();
	dequeue_and_wake(dongle);
	pthread_mutex_unlock(&dongle->mutex);
}

/*
Lock both dongles and unlock the first one when the second one
is not successfully locked.
Return 1 if both dongles are successfully locked, 0 otherwise.
*/
static int	acquire_dongles(t_coder *coder, t_dongle *dongle1,
	t_dongle *dongle2)
{
	if (lock_dongle(coder, dongle1) == 0)
		return (0);
	if (coder->args->num_coders == 1)
	{
		unlock_dongle(dongle1);
		return (0);
	}
	if (lock_dongle(coder, dongle2) == 0)
	{
		unlock_dongle(dongle1);
		return (0);
	}
	return (1);
}

/*
Compile by locking 2 dongles in the order of odd-idx -> even-idx dongle.
Return 1 if the coder successfully compiled, 0 otherwise.
*/
int	compile(t_coder *coder)
{
	if (coder->args->num_coders == 1)
		return (acquire_dongles(coder, coder->rdongle, NULL));
	if (coder->id % 2 == 0)
	{
		if (acquire_dongles(coder, coder->ldongle, coder->rdongle) == 0)
			return (0);
	}
	else
	{
		if (acquire_dongles(coder, coder->rdongle, coder->ldongle) == 0)
			return (0);
	}
	pthread_mutex_lock(coder->burnout_lock);
	coder->last_compile = get_current_time();
	pthread_mutex_unlock(coder->burnout_lock);
	print_status(coder, "is compiling");
	usleep(coder->args->time_compile * 1000);
	pthread_mutex_lock(coder->compiles_lock);
	coder->already_compiled++;
	pthread_mutex_unlock(coder->compiles_lock);
	unlock_dongle(coder->ldongle);
	unlock_dongle(coder->rdongle);
	return (1);
}
