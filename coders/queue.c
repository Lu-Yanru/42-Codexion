/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 17:21:11 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/27 15:22:23 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Set the priority queue based on scheduler.
For fifo: lock and unlock queue_lock.
For edf: lock and unlock burnout_lock.
Lock and unlock queue_lock.
*/
void	enqueue(t_coder *coder, t_dongle *dongle)
{
	t_queue_node	my_node;

	my_node.coder_id = coder->id;
	pthread_mutex_lock(&dongle->queue_lock);
	if (coder->args->scheduler == 0)
		my_node.priority = dongle->ticket++;
	else
	{
		pthread_mutex_lock(coder->burnout_lock);
		my_node.priority = coder->last_compile + coder->args->time_burnout;
		pthread_mutex_unlock(coder->burnout_lock);
	}
	// printf("coder: %d priority: %lu enqueued\n", coder->id, my_node.priority);
	push(&dongle->queue, my_node);
	// printf("heap state:\n");
	// printf("1st position: coder: %d, priority: %lu\n", dongle->queue.queue[0].coder_id, dongle->queue.queue[0].priority);
	// printf("2nd position: coder: %d, priority: %lu\n", dongle->queue.queue[1].coder_id, dongle->queue.queue[1].priority);
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->queue_lock);
}

void	enqueue_both(t_coder *coder, t_dongle *dongle1, t_dongle *dongle2)
{
	enqueue(coder, dongle1);
	enqueue(coder, dongle2);
}

/*
Dequeue the first coder and wake the other coders in the queue.
Lock and unlock queue_lock.
*/
void	dequeue(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->queue_lock);
	pop(&dongle->queue);
	// printf("dequeued. heap state:\n");
	// printf("1st position: coder: %d, priority: %lu\n", dongle->queue.queue[0].coder_id, dongle->queue.queue[0].priority);
	// printf("2nd position: coder: %d, priority: %lu\n", dongle->queue.queue[1].coder_id, dongle->queue.queue[1].priority);
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->queue_lock);
}

void	dequeue_both(t_dongle *dongle1, t_dongle *dongle2)
{
	dequeue(dongle1);
	dequeue(dongle2);
}
