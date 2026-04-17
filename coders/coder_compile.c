/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_compile.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 15:56:40 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/17 19:27:35 by yanlu            ###   ########.fr       */
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
Set the flag_is_compiling to a given value within burnout_lock.
*/
static void	set_is_compiling_flag(t_coder *coder, int flag)
{
	pthread_mutex_lock(coder->burnout_lock);
	coder->flag_is_compiling = flag;
	pthread_mutex_unlock(coder->burnout_lock);
}

/*
Record the time when the dongle was used
and unlock the dongle.
*/
static void	unlock_dongle(t_coder *coder, t_dongle *dongle)
{
	set_is_compiling_flag(coder, 0);
	dongle->last_used = get_current_time();
	pthread_mutex_unlock(&dongle->mutex);
}

/*
Compile by locking 2 dongles in the order of odd-idx -> even-idx dongle.
When compiling, set the flag_is_compiling to 1
and set back to 0 when compiling finished.
Return 1 if the coder successfully compiled, 0 otherwise.
*/
int	compile(t_coder *coder)
{
	set_is_compiling_flag(coder, 1);
	pthread_mutex_lock(coder->burnout_lock);
	coder->last_compile = get_current_time();
	pthread_mutex_unlock(coder->burnout_lock);
	if (coder->id % 2 == 0)
	{
		if (lock_dongle(coder, coder->ldongle) == 0)
		{
			set_is_compiling_flag(coder, 0);
			return (0);
		}
		if (lock_dongle(coder, coder->rdongle) == 0)
		{
			unlock_dongle(coder, coder->ldongle);
			return (0);
		}
	}
	else
	{
		if (lock_dongle(coder, coder->rdongle) == 0)
		{
			set_is_compiling_flag(coder, 0);
			return (0);
		}
		if (coder->args->num_coders == 1)
		{
			unlock_dongle(coder, coder->rdongle);
			return (0);
		}
		if (lock_dongle(coder, coder->ldongle) == 0)
		{
			unlock_dongle(coder, coder->rdongle);
			return (0);
		}
	}
	print_status(coder, "is compiling");
	usleep(coder->args->time_compile * 1000);
	pthread_mutex_lock(coder->compiles_lock);
	coder->already_compiled++;
	pthread_mutex_unlock(coder->compiles_lock);
	unlock_dongle(coder, coder->ldongle);
	unlock_dongle(coder, coder->rdongle);
	return (1);
}
