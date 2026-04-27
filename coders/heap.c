/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:39:46 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/27 18:52:52 by yanlu            ###   ########.fr       */
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
void	push(t_queue *queue, t_queue_node node)
{
	if (queue->size >= 2)
		return ;
	queue->queue[queue->size] = node;
	queue->size++;
	if (queue->size == 2
		&& (queue->queue[1].priority < queue->queue[0].priority))
		swap(queue);
}

/*
Remove the first coder ticket/ddl from the queue for a dongle
and move the second coder to the first slot.
*/
void	pop(t_queue *queue)
{
	if (queue->size <= 0)
		return ;
	queue->queue[0] = queue->queue[1];
	queue->size--;
	queue->queue[queue->size].coder_id = 0;
	queue->queue[queue->size].priority = 0;
}
