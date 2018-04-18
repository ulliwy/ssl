/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   md5_prep.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iprokofy <iprokofy@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/04/17 14:34:02 by iprokofy          #+#    #+#             */
/*   Updated: 2018/04/18 16:23:34 by iprokofy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ssl.h"

int		md5_err_usage()
{
	ft_putstr("usage: ft_ssl md5 [-pqr] [-s string] [files ...]\n");
	return (1);
}

int		md5_err_options(char opt)
{
	ft_putstr("ft_ssl md5: illegal option -- ");
	ft_putchar(opt);
	ft_putstr("\n");
	return (0);
}

void			md5(char *msg, ssize_t size, t_md5 *opts)
{
	int bit_size;
	int new_size;
	unsigned char *new;

	new = (unsigned char*)msg;
	bit_size = size * 8;
	new_size = ((bit_size + 64) / 512) * 512 + 448;

	printf("%lu %d %d\n", size, bit_size, new_size);

	if (opts->s)
	{
		new = (unsigned char*)ft_memalloc(new_size / 8);
		ft_strcpy((char*)new, msg);
	}

	new[size] = 128;
	
	size = 0;
	int i = 0;
	while (i < new_size / 8)
	{
		if (i && i % 8 == 0)
			ft_putchar('\n');
		if (i % 8 == 0)
		{
			ft_putnbr((i * 8) / 512);
			ft_putchar(' ');
		}
		print_bits(new[i], 8);
		ft_putchar(' ');
		msg++;
		i++;
	}
	if (opts->s)
	{
		free(new);
		opts->s = 0;
	}
	return;
}

static int		parse_opts(char **av, int i, t_md5 *opts)
{
	if (av[i][0] == '-' && !av[i][2])
	{
		if (av[i][1] == 'q')
			opts->q = 1;
		else if (av[i][1] == 'r')
			opts->r = 1;
		else if (av[i][1] == 's' && av[i + 1])
		{
			opts->s = 1;
			md5(av[i + 1], (ssize_t)ft_strlen(av[i + 1]), opts);
			i++;
		}
		else if (av[i][1] == 'p')
			opts->stdinn = get_from_fd(0, &(opts->in_size));
		else
			return (md5_err_options(av[i][1]));
	}
	else
		return (0);
	return (i + 1);
}

void	md5_opts_init(t_md5 *opts)
{
	opts->q = 0;
	opts->r = 0;
	opts->s = 0;
	opts->stdinn = NULL;
}

int		md5_prep(int argc, char **argv)
{
	int 	i;
	t_md5	opts;

	md5_opts_init(&opts);
	i = 2;
	while (i < argc)
	{
		if (!(i = parse_opts(argv, i, &opts)))
			return (md5_err_usage());
	}
	return (0);
}
