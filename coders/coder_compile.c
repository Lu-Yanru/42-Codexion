/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_compile.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 12:25:08 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/27 16:51:08 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Check if a coder should yield and requeue:
When a coder is:
- at the front of dongle2 but not at the front of dongle1
- the coder behind in dongle2's queue has the same priority
-> prevents deadlock when same priority
Lock and unlock queue_lock of both dongles.
Return 1 if yes, 0 no.
*/
static int	should_yield(t_coder *coder, t_dongle *dongle1, t_dongle *dongle2)
{
	int	res;

	pthread_mutex_lock(&dongle1->queue_lock);
	pthread_mutex_lock(&dongle2->queue_lock);
	if (dongle2->queue.size < 2)
	{
		pthread_mutex_unlock(&dongle2->queue_lock);
		pthread_mutex_unlock(&dongle1->queue_lock);
		return (0);
	}
	if (dongle2->queue.queue[0].coder_id != coder->id)
	{
		pthread_mutex_unlock(&dongle2->queue_lock);
		pthread_mutex_unlock(&dongle1->queue_lock);
		return (0);
	}
	if (dongle1->queue.queue[0].coder_id == coder->id)
	{
		pthread_mutex_unlock(&dongle2->queue_lock);
		pthread_mutex_unlock(&dongle1->queue_lock);
		return (0);
	}
	if (dongle2->queue.queue[0].priority == dongle2->queue.queue[1].priority)
		res = 1;
	else
		res = 0;
	pthread_mutex_unlock(&dongle2->queue_lock);
	pthread_mutex_unlock(&dongle1->queue_lock);
	return (res);
}

/*
Check if a coder is at the front of a queue protected by queue_lock.
Return 1 if yes, 0 no.
*/
static int	check_front(t_coder *coder, t_dongle *dongle)
{
	int	res;

	pthread_mutex_lock(&dongle->queue_lock);
	if (dongle->queue.queue[0].coder_id == coder->id)
		res = 1;
	else
		res = 0;
	pthread_mutex_unlock(&dongle->queue_lock);
	return (res);
}

/*
Wait until coder is at the front of both dongles.
Wait until coder is at the front of the queue of dongle1.
Check if the coder is also at the front of dongle2 queue.
If so, proceed (return 1).
Otherwise, check if the coder at the front of dongle2
is also waiting for their dongle1,
if so, yield (dequeue and reenqueue).
To prevent both coders yield to each other infinitvely,
keep a yield counter and each time sleep a bit longer before re-yielding.
Otherwise keep waiting.
Return 0 if the monitor signals stop while waiting.
Both dongle mutexes held on return if successful,
otherwise unlock both dongle mutexes.
*/
int	wait_for_both_fronts(t_coder *coder, t_dongle *dongle1,
	t_dongle *dongle2)
{
	// struct timespec	ts;
	// unsigned long	wait_time;

	while (1)
	{
		if (check_stop(coder))
		{
			dequeue_both(dongle1, dongle2);
			return (0);
		}
		if (should_yield(coder, dongle1, dongle2))
		{
			dequeue(dongle2);
			usleep(coder->yield_count * 100);
			coder->yield_count++;
			enqueue(coder, dongle2);
			continue ;
		}
		if (check_front(coder, dongle1) && check_front(coder, dongle2))
		{
			pthread_mutex_lock(&dongle1->mutex);
			pthread_mutex_lock(&dongle2->mutex);
			if (check_front(coder, dongle1) && check_front(coder, dongle2))
				return (1);
			pthread_mutex_unlock(&dongle2->mutex);
			pthread_mutex_unlock(&dongle1->mutex);
		}
		pthread_mutex_lock(&dongle1->queue_lock);
		pthread_cond_wait(&dongle1->cond, &dongle1->queue_lock);
		pthread_mutex_unlock(&dongle1->queue_lock);
	}
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
	unsigned long	sleep_time;

	while (1)
	{
		now = get_current_time();
		if (now >= dongle1->ready_time && now >= dongle2->ready_time)
			break ;
		if (check_stop(coder))
		{
			dequeue_both(dongle1, dongle2);
			pthread_mutex_unlock(&dongle2->mutex);
			pthread_mutex_unlock(&dongle1->mutex);
			return (0);
		}
		if (dongle2->ready_time > dongle1->ready_time)
			wake_time = dongle2->ready_time;
		else
			wake_time = dongle1->ready_time;
		sleep_time = wake_time - now;
		if (sleep_time > 0)
		{
			pthread_mutex_unlock(&dongle2->mutex);
			pthread_mutex_unlock(&dongle1->mutex);
			usleep(sleep_time * 1000);
			pthread_mutex_lock(&dongle1->mutex);
			pthread_mutex_lock(&dongle2->mutex);
		}
		if (check_stop(coder))
		{
			dequeue_both(dongle1, dongle2);
			pthread_mutex_unlock(&dongle2->mutex);
			pthread_mutex_unlock(&dongle1->mutex);
			return (0);
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
		pthread_mutex_unlock(&dongle2->mutex);
		pthread_mutex_unlock(&dongle1->mutex);
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
	// pthread_cond_broadcast(&dongle->cond);
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
