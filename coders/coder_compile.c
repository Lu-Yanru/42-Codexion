/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_compile.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 15:56:40 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/21 17:56:54 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Lock the dongle if the cooldown time has passed.
*/
static int	lock_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	while (get_current_time()
			< dongle->last_used + coder->args->dongle_cooldown)
	{
		pthread_mutex_unlock(&dongle->mutex);
		if (check_stop(coder))
			return (0);
		usleep(100);
		pthread_mutex_lock(&dongle->mutex);
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
