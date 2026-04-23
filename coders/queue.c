/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 17:21:11 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/23 17:22:34 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Set the priority queue based on scheduler.
*/
void	enqueue(t_coder *coder, t_dongle *dongle)
{
	t_queue_node	my_node;

	my_node.coder_id = coder->id;
	if (coder->args->scheduler == 0)
		my_node.priority = dongle->ticket++;
	else
		my_node.priority = coder->last_compile + coder->args->time_burnout;
	push(&dongle->queue, my_node);
}

void	enqueue_both(t_coder *coder, t_dongle *dongle1, t_dongle *dongle2)
{
	pthread_mutex_lock(&dongle1->mutex);
	pthread_mutex_lock(&dongle2->mutex);
	enqueue(coder, dongle1);
	enqueue(coder, dongle2);
	pthread_mutex_unlock(&dongle2->mutex);
	pthread_mutex_unlock(&dongle1->mutex);
}

/*
Dequeue the first coder and wake the other coders in the queue.
*/
void	dequeue(t_dongle *dongle)
{
	pop(&dongle->queue);
	pthread_cond_broadcast(&dongle->cond);
}

void	dequeue_both(t_dongle *dongle1, t_dongle *dongle2)
{
	dequeue(dongle1);
	pthread_mutex_unlock(&dongle1->mutex);
	dequeue(dongle2);
	pthread_mutex_unlock(&dongle2->mutex);
}
