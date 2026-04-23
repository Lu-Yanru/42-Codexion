/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_compile.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 12:25:08 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/23 17:15:15 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Wait until the coder is at the front of the dongle queue.
Returns 0 if the stop flag is set, 1 otherwise.
Assumes mutex is held on entry and remains held on return.
*/
static int	wait_for_front(t_coder *coder, t_dongle *dongle)
{
	// printf("dongle1 queue front: %d\n", dongle->queue.queue[0].coder_id);
	while (dongle->queue.queue[0].coder_id != coder->id)
	{
		// printf("dongle1 queue front: %d\n", dongle->queue.queue[0].coder_id);
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
		if (check_stop(coder))
		{
			dequeue(dongle);
			return (0);
		}
	}
	return (1);
}

/*
Wait until coder is at the front of both dongles.
Wait until coder is at the front of the queue of dongle1.
Check if the coder is also at the front of dongle2 queue.
If so, proceed (return 1), otherwise keep waiting.
Return 0 if the monitor signals stop while waiting.
Both mutexes held on return if successful.
*/
int	wait_for_both_fronts(t_coder *coder, t_dongle *dongle1,
	t_dongle *dongle2)
{
	while (1)
	{
		pthread_mutex_lock(&dongle1->mutex);
		if (!wait_for_front(coder, dongle1))
		{
			pthread_mutex_unlock(&dongle1->mutex);
			pthread_mutex_lock(&dongle2->mutex);
			dequeue(dongle2);
			pthread_mutex_unlock(&dongle2->mutex);
			return (0);
		}
		pthread_mutex_lock(&dongle2->mutex);
		// printf("dongle2 queue front: %d\n", dongle2->queue.queue[0].coder_id);
		if (dongle2->queue.queue[0].coder_id == coder->id)
			break ;
		// printf("dongle2 queue front: %d\n", dongle2->queue.queue[0].coder_id);
		if (check_stop(coder))
		{
			dequeue_both(dongle1, dongle2);
			return (0);
		}
		pthread_mutex_unlock(&dongle2->mutex);
		pthread_mutex_unlock(&dongle1->mutex);
		usleep(100);
	}
	return (1);
}

/*
Wait for cooldown on both dongles.
Assumes both mutexes are held on entry.
Returns 0 if stop flag is set, 1 otherwise.
Both mutexes remain held on return if successful.
*/
static int	wait_for_cooldown(t_coder *coder, t_dongle *dongle1,
	t_dongle *dongle2)
{
	unsigned long	now;
	unsigned long	wake_time;
	unsigned long	dongle1_ready_time;
	unsigned long	dongle2_ready_time;
	unsigned long	sleep_time;

	while (1)
	{
		now = get_current_time();
		// printf("now: %lu, dongle1 last_used: %lu, dongle2 last_used: %lu\n", now, dongle1->last_used, dongle2->last_used);
		dongle1_ready_time = dongle1->last_used + coder->args->dongle_cooldown;
		dongle2_ready_time = dongle2->last_used + coder->args->dongle_cooldown;
		if (now >= dongle1_ready_time && now >= dongle2_ready_time)
			break ;
		if (check_stop(coder))
		{
			dequeue_both(dongle1, dongle2);
			return (0);
		}
		if (dongle2_ready_time > dongle1_ready_time)
			wake_time = dongle2_ready_time;
		else
			wake_time = dongle1_ready_time;
		sleep_time = wake_time - now;
		if (sleep_time > 0)
		{
			pthread_mutex_unlock(&dongle2->mutex);
			pthread_mutex_unlock(&dongle1->mutex);
			usleep(sleep_time * 1000);
			pthread_mutex_lock(&dongle1->mutex);
			pthread_mutex_lock(&dongle2->mutex);
		}
	}
	return (1);
}

/*
Lock both dongles in queue order with cooldown check.
Return 1 if successfully locked both dongle, 0 otherwise.
*/
static int	lock_both_dongles(t_coder *coder, t_dongle *dongle1,
	t_dongle *dongle2)
{
	if (coder->args->num_coders == 1)
	{
		pthread_mutex_lock(&dongle1->mutex);
		print_status(coder, "has taken a dongle");
		pthread_mutex_unlock(&dongle1->mutex);
		return (0);
	}
	enqueue_both(coder, dongle1, dongle2);
	while (1)
	{
		if (!wait_for_both_fronts(coder, dongle1, dongle2))
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
		dequeue_both(dongle1, dongle2);
		return (0);
	}
	print_status(coder, "has taken a dongle");
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
		if (lock_both_dongles(coder, coder->ldongle, coder->rdongle) == 0)
			return (0);
	}
	else
	{
		if (lock_both_dongles(coder, coder->rdongle, coder->ldongle) == 0)
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
