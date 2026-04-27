/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_compile_wait_front.c                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 17:54:49 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/27 18:52:26 by yanlu            ###   ########.fr       */
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
	if (dongle2->queue.size < 2
		|| dongle2->queue.queue[0].coder_id != coder->id
		|| dongle1->queue.queue[0].coder_id == coder->id)
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
Dequeue and re-enqueue the coder from dongle2.
*/
static void	do_yield(t_coder *coder, t_dongle *dongle2)
{
	dequeue(dongle2);
	usleep(coder->yield_count * 100);
	coder->yield_count++;
	enqueue(coder, dongle2);
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
Try to lock both dongle mutexes.
Return 1 after re-checking that the coder is
at the front of both dongles.
Return 0 if not and unlocks both dongle mutexes again.
*/
static int	try_acquire_dongles(t_coder *coder, t_dongle *dongle1,
	t_dongle *dongle2)
{
	pthread_mutex_lock(&dongle1->mutex);
	pthread_mutex_lock(&dongle2->mutex);
	if (check_front(coder, dongle1) && check_front(coder, dongle2))
		return (1);
	pthread_mutex_unlock(&dongle2->mutex);
	pthread_mutex_unlock(&dongle1->mutex);
	return (0);
}

/*
Wait until coder is at the front of both dongles.
Preiodically checks
(when monitoring thread (recheck_queue())
or enqueueing or dequeueing broadcasts):
- If the stop flag has been set -> return 0
- If the coder should yield
- If the coder is at the front of both dongles
	-> locks both dongle mutexex and return 1
To prevent both coders yield to each other infinitvely,
keep a yield counter and each time sleep a bit longer before re-yielding.
Otherwise keep waiting.
Both dongle mutexes held on return if successful,
otherwise unlock both dongle mutexes.
*/
int	wait_for_fronts(t_coder *coder, t_dongle *dongle1, t_dongle *dongle2)
{
	while (1)
	{
		if (check_stop(coder))
		{
			dequeue_both(dongle1, dongle2);
			return (0);
		}
		if (should_yield(coder, dongle1, dongle2))
		{
			do_yield(coder, dongle2);
			continue ;
		}
		if (check_front(coder, dongle1) && check_front(coder, dongle2))
		{
			if (try_acquire_dongles(coder, dongle1, dongle2))
				return (1);
		}
		pthread_mutex_lock(&dongle1->queue_lock);
		pthread_cond_wait(&dongle1->cond, &dongle1->queue_lock);
		pthread_mutex_unlock(&dongle1->queue_lock);
	}
}
