/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_compile.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 12:25:08 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/27 18:54:55 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	one_coder(t_coder *coder)
{
	pthread_mutex_lock(&coder->rdongle->mutex);
	print_status(coder, "has taken a dongle");
	pthread_mutex_unlock(&coder->rdongle->mutex);
	return (0);
}

/*
Lock both dongles in queue order with cooldown check.
Return 1 if successfully locked both dongle, 0 otherwise.
*/
static int	lock_both_dongles(t_coder *coder, t_dongle *dongle1,
	t_dongle *dongle2)
{
	if (coder->args->num_coders == 1)
		return (one_coder(coder));
	enqueue_both(coder, dongle1, dongle2);
	while (1)
	{
		if (!wait_for_fronts(coder, dongle1, dongle2))
			return (0);
		if (!wait_for_cooldown(coder, dongle1, dongle2))
			return (0);
		if (dongle1->queue.queue[0].coder_id == coder->id
			&& dongle2->queue.queue[0].coder_id == coder->id)
			break ;
		pthread_mutex_unlock(&dongle2->mutex);
		pthread_mutex_unlock(&dongle1->mutex);
	}
	if (check_stop(coder))
	{
		dequeue_and_unlock(dongle1, dongle2);
		return (0);
	}
	print_status(coder, "has taken a dongle");
	print_status(coder, "has taken a dongle");
	return (1);
}

/*
Record the time when the dongle was used,
reset yield_count, dequeue the coder
and unlock the dongle.
*/
static void	unlock_dongle(t_coder *coder, t_dongle *dongle)
{
	dongle->ready_time = get_current_time() + dongle->args->dongle_cooldown;
	coder->yield_count = 0;
	dequeue(dongle);
	pthread_mutex_unlock(&dongle->mutex);
}

/*
Compile by locking 2 dongles in the order of odd-idx -> even-idx dongle.
Return 1 if the coder successfully compiled, 0 otherwise.
*/
int	compile(t_coder *coder)
{
	if (coder->args->num_coders == 1)
		return (lock_both_dongles(coder, coder->rdongle, NULL));
	if (coder->id % 2 == 0)
	{
		if (!lock_both_dongles(coder, coder->ldongle, coder->rdongle))
			return (0);
	}
	else
	{
		if (!lock_both_dongles(coder, coder->rdongle, coder->ldongle))
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
	unlock_dongle(coder, coder->ldongle);
	unlock_dongle(coder, coder->rdongle);
	return (1);
}
