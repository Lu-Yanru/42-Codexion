/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_input.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanlu <yanlu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 17:00:58 by yanlu             #+#    #+#             */
/*   Updated: 2026/04/07 17:29:18 by yanlu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_number(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	is_scheduler(char *str)
{
	if ((strcmp(str, "fifo") != 0) && (strcmp(str, "edf") != 0))
		return (0);
	return (1);
}

/*
A function that validates the CLI.
Returns 1 if all arguments are valid.
0 if not.
*/
int	check_input(int argc, char *argv[])
{
	int	i;

	i = 1;
	if (argc != 9)
		return (0);
	while (i < argc - 1)
	{
		if (is_number(argv[i]) == 0)
			return (0);
		i++;
	}
	if (is_scheduler(argv[i]) == 0)
		return (0);
	return (1);
}
