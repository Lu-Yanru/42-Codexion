/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 17:21:11 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/24 15:40:41 by yanlu            ###   ########.fr       */
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
	{
		pthread_mutex_lock(&dongle->queue_lock);
		my_node.priority = dongle->ticket++;
		pthread_mutex_unlock(&dongle->queue_lock);
	}
	else
	{
		pthread_mutex_lock(coder->burnout_lock);
		my_node.priority = coder->last_compile + coder->args->time_burnout;
		pthread_mutex_unlock(coder->burnout_lock);
	}
	pthread_mutex_lock(&dongle->queue_lock);
	printf("coder: %d priority: %lu enqueued\n", coder->id, my_node.priority);
	push(&dongle->queue, my_node);
	printf("heap state:\n");
	printf("1st position: coder: %d, priority: %lu\n", dongle->queue.queue[0].coder_id, dongle->queue.queue[0].priority);
	printf("2nd position: coder: %d, priority: %lu\n", dongle->queue.queue[1].coder_id, dongle->queue.queue[1].priority);
	pthread_mutex_unlock(&dongle->queue_lock);
}

void	enqueue_both(t_coder *coder, t_dongle *dongle1, t_dongle *dongle2)
{
	// pthread_mutex_lock(&dongle1->mutex);
	// pthread_mutex_lock(&dongle2->mutex);
	enqueue(coder, dongle1);
	enqueue(coder, dongle2);
	// pthread_mutex_unlock(&dongle2->mutex);
	// pthread_mutex_unlock(&dongle1->mutex);
}

/*
Dequeue the first coder and wake the other coders in the queue.
*/
void	dequeue(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->queue_lock);
	pop(&dongle->queue);
	printf("dequeued. heap state:\n");
	printf("1st position: coder: %d, priority: %lu\n", dongle->queue.queue[0].coder_id, dongle->queue.queue[0].priority);
	printf("2nd position: coder: %d, priority: %lu\n", dongle->queue.queue[1].coder_id, dongle->queue.queue[1].priority);
	pthread_mutex_unlock(&dongle->queue_lock);
	pthread_cond_broadcast(&dongle->cond);
}

void	dequeue_both(t_dongle *dongle1, t_dongle *dongle2)
{
	dequeue(dongle1);
	pthread_mutex_unlock(&dongle1->mutex);
	dequeue(dongle2);
	pthread_mutex_unlock(&dongle2->mutex);
}
