/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   valid_file.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apigeon <apigeon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/12 19:08:54 by apigeon           #+#    #+#             */
/*   Updated: 2022/12/13 17:57:11 by apigeon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser.h"

static bool	valid_filename(const char *filename)
{
	int	len;

	if (!filename)
		return (false);
	len = ft_strlen(filename);
	if (len < 5)
		return (false);
	if (ft_strcmp(filename + (len - 3 - 1), ".cub") != 0)
		return (false);
	return (true);
}

static bool	file_readable(const char *filename)
{
	int	fd;

	fd = open(filename, O_RDONLY);
	if (fd == -1)
		return (false);
	if (read(fd, NULL, 0) == -1)
		return (close(fd), false);
	close(fd);
	return (true);
}

/* Validate if a file is a .cub file, and also checks
 * if it exists and is readable,
 * filename: the path to the file to check,
 * returns: true if the filename is valid, false otherwise
 */
bool	valid_file(const char *filename)
{
	if (!valid_filename(filename))
		return (err_message(MSG_INVALID_FILENAME), false);
	if (!file_readable(filename))
		return (err_message(MSG_FILE_NOT_READABLE), false);
	return (true);
}
