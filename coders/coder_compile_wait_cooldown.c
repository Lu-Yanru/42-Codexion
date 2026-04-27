/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_compile_wait_cooldown.c                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 18:32:30 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/27 18:55:16 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Get the timestamp when the coder should wake up again,
i.e. when both dongles have cooled down.
*/
static unsigned long	get_wake_time(t_dongle *dongle1, t_dongle *dongle2)
{
	if (dongle2->ready_time > dongle1->ready_time)
		return (dongle2->ready_time);
	return (dongle1->ready_time);
}

/*
Unlock both dongle mutexes, sleep for sleep_time,
then lock both dongle mutexes again.
Return 1 if successful, 0 if stop_flag is set.
*/
static int	sleep_for_cooldown(t_coder *coder, t_dongle *dongle1,
	t_dongle *dongle2, unsigned long sleep_time)
{
	pthread_mutex_unlock(&dongle2->mutex);
	pthread_mutex_unlock(&dongle1->mutex);
	usleep(sleep_time * 1000);
	pthread_mutex_lock(&dongle1->mutex);
	pthread_mutex_lock(&dongle2->mutex);
	if (check_stop(coder))
	{
		dequeue_and_unlock(dongle1, dongle2);
		return (0);
	}
	return (1);
}

/*
Wait for cooldown on both dongles.
Assumes both mutexes are held on entry.
Returns 0 if stop flag is set, 1 otherwise.
Both mutexes remain held on return if successful.
*/
int	wait_for_cooldown(t_coder *coder, t_dongle *dongle1, t_dongle *dongle2)
{
	unsigned long	now;
	unsigned long	sleep_time;

	while (1)
	{
		now = get_current_time();
		if (now >= dongle1->ready_time && now >= dongle2->ready_time)
			break ;
		if (check_stop(coder))
		{
			dequeue_and_unlock(dongle1, dongle2);
			return (0);
		}
		sleep_time = get_wake_time(dongle1, dongle2) - now;
		if (sleep_time > 0)
		{
			if (!sleep_for_cooldown(coder, dongle1, dongle2, sleep_time))
				return (0);
		}
	}
	return (1);
}
