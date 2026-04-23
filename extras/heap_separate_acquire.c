/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:39:46 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/22 14:57:44 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
Swap the two coders in a queue for a dongle.
*/
static void	swap(t_queue *queue)
{
	t_queue_node	tmp;

	tmp = queue->queue[0];
	queue->queue[0] = queue->queue[1];
	queue->queue[1] = tmp;
}

/*
Add a coder ticket/ddl to the queue for a dongle.
*/
void	enqueue(t_queue *queue, t_queue_node node)
{
	queue->queue[queue->size] = node;
	queue->size++;
	if (queue->size == 2
			&& queue->queue[1].priority < queue->queue[0].priority)
		swap(queue);
}

/*
Remove the first coder ticket/ddl from the queue for a dongle
and move the second coder to the first slot.
*/
void	dequeue(t_queue *queue)
{
	queue->queue[0] = queue->queue[1];
	queue->size--;
}

/*
Dequeue the first coder and wake the other coders in the queue.
*/
void	dequeue_and_wake(t_dongle *dongle)
{
	dequeue(&dongle->queue);
	pthread_cond_broadcast(&dongle->cond);
}
