/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   md5_prep.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Ulliwy <Ulliwy@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/04/17 14:34:02 by iprokofy          #+#    #+#             */
/*   Updated: 2018/07/05 15:32:07 by Ulliwy           ###   ########.fr       */
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

void add_size_representation(unsigned char *msg, ssize_t size, ssize_t new_size)
{
	msg[new_size / 8 - 1] = (char)(size >> (8 * 7));
	msg[new_size / 8 - 2] = (char)(size >> (8 * 6));
	msg[new_size / 8 - 3] = (char)(size >> (8 * 5));
	msg[new_size / 8 - 4] = (char)(size >> (8 * 4));
	msg[new_size / 8 - 5] = (char)(size >> (8 * 3));
	msg[new_size / 8 - 6] = (char)(size >> (8 * 2));
	msg[new_size / 8 - 7] = (char)(size >> (8 * 1));
	msg[new_size / 8 - 8] = (char)(size >> (8 * 0));
}

uint32_t *init_buffer()
{
	uint32_t *buf;

	buf = (uint32_t *)malloc(4 * sizeof(uint32_t));
	buf[0] = 0x67452301;
	buf[1] = 0xefcdab89;
	buf[2] = 0x98badcfe;
	buf[3] = 0x10325476;
	return (buf);
}

uint32_t left_rotate(uint32_t x, int c)
{
	return (x << c) | (x >> (32 - c));
}

void	process_chunk(unsigned char *chunk, uint32_t *buffer) // 16 words
{
	uint32_t *words;
	t_md5_buf buf;
	int i;
	uint32_t F;
	uint32_t g;

	words = (uint32_t *)chunk;

	buf.a = buffer[0];
	buf.b = buffer[1];
	buf.c = buffer[2];
	buf.d = buffer[3];

	i = 0;
	while (i < 64)
	{
		if (i < 16)
		{
			F = (buf.b & buf.c) | (~(buf.b) & buf.d);
			g = i;
		}
		else if (i < 32)
		{
			F = (buf.d & buf.b) | (~(buf.d) & buf.c);
			g = (5 * i + 1) % 16;
		}
		else if (i < 48)
		{
			F = buf.b ^ buf.c ^ buf.d;
			g = (3 * i + 5) % 16;
		}
		else if (i < 64)
		{
			F = buf.c ^ (buf.b | ~(buf.d));
			g = (7 * i) % 16;
		}
		F = F + buf.a + g_md5_K[i] + words[g];
		buf.a = buf.d;
		buf.d = buf.c;
		buf.c = buf.b;
		buf.b = buf.b + left_rotate(F, g_md5_s[i]);
		//printf("%d A: %u, B: %u, C: %u, D: %u\n", i, buf.a, buf.b, buf.c, buf.d);
		i++;
	}
	buffer[0] = buffer[0] + buf.a;
	buffer[1] = buffer[1] + buf.b;
	buffer[2] = buffer[2] + buf.c;
	buffer[3] = buffer[3] + buf.d;
}

unsigned char *pad_msg(unsigned char *msg, ssize_t *size)
{
	ssize_t bit_size;
	ssize_t new_size;
	unsigned char *new_msg;

	bit_size = (*size) * 8;
	new_size = ((bit_size + 64) / 512) * 512 + 448 + 8 * 8; // in bits
	new_msg = (unsigned char *)ft_memalloc(new_size / 8 + 8);
	ft_strcpy((char *)new_msg, (char *)msg);
	new_msg[*size] = 128;
	//printf("%lu %lu\n", *size, new_size);
	add_size_representation(new_msg, *size * 8, new_size);
	*size = new_size / 8;
	//free(msg);
	return (new_msg);
}

void	print_hex(uint32_t c)
{
	char	base[17] = "0123456789abcdef";

	ft_putchar(base[c / 16]);
	ft_putchar(base[c % 16]);
}

void	print_hash(uint32_t *buffer, t_md5 *opts)
{
	int i = 0;

	if (opts->p)
		write(1, opts->input, opts->input_size);

	if (opts->str && !opts->q)
	{
		write(1, "MD5 (\"", 6);
		write(1, opts->str, opts->str_len);
		write(1, "\") = ", 5);
	}
	else if (!opts->q && opts->filename)
	{
		write(1, "MD5 (", 5);
		write(1, opts->filename, ft_strlen(opts->filename));
		write(1, ") = ", 4);
	}

	while (i < 4)
	{
		print_hex(buffer[i] & 0xFF);
		print_hex((buffer[i] & 0xFF00) >> 8);
		print_hex((buffer[i] & 0xFF0000) >> 8 * 2);
		print_hex((buffer[i] & 0xFF000000) >> 8 * 3);
		i++;
	}
	printf("\n");
}

void	md5(unsigned char *msg, ssize_t size, t_md5 *opts)
{
	int i;
	uint32_t *buffer;

	msg = pad_msg(msg, &size);
	buffer = init_buffer();
	i = 0;
	while (i < size / 64)
	{
		process_chunk(msg + i * 64, buffer);
		i++;
	}
	print_hash(buffer, opts);
}

static int		parse_opts(char **av, int i, t_md5 *opts, int *parsed)
{
	if (av[i][0] == '-' && !av[i][2])
	{
		if (av[i][1] == 'q')
		{
			opts->q = 1;
			opts->input = (unsigned char *)get_from_fd(0, &(opts->input_size));
			md5(opts->input, opts->input_size, opts);
			free(opts->input);
		}
		else if (av[i][1] == 'r')
			opts->r = 1;
		else if (av[i][1] == 's' && av[i + 1])
		{
			opts->str = (unsigned char *)av[i + 1];
			opts->str_len = (ssize_t)ft_strlen(av[i + 1]);
			md5(opts->str, opts->str_len, opts);
			opts->str = NULL;
			i++;
		}
		else if (av[i][1] == 'p')
		{
			opts->p = 1;
			opts->input = (unsigned char *)get_from_fd(0, &(opts->input_size));
			md5(opts->input, opts->input_size, opts);
			free(opts->input);
		}
		else
			return (md5_err_options(av[i][1]));
	}
	else
	{
		*parsed = i;
		return (1);
	}
	return (i + 1);
}

void	md5_opts_init(t_md5 *opts)
{
	opts->q = 0;
	opts->r = 0;
	//opts->s = 0;
	opts->p = 0;
	opts->filename = NULL;

	opts->str = NULL;
	//opts->str_len = 0;

	opts->input = NULL;
	opts->input_size = 0;
}

void	hash_file(char *file_name, t_md5 *opts)
{
	int fd;

	fd = open(file_name, O_RDONLY);
	if (fd == -1)
		return (hash_open_err(file_name, "md5", 3));
	opts->input = (unsigned char *)get_from_fd(fd, &(opts->input_size));
	md5(opts->input, opts->input_size, opts);
	free(opts->input);
	close(fd);
}

int		md5_prep(int argc, char **argv)
{
	int 	i;
	t_md5	opts;
	int 	parsed;

	md5_opts_init(&opts);
	i = 2;
	parsed = 0;
	while (i < argc && !parsed)
	{
		i = parse_opts(argv, i, &opts, &parsed);
		if (i == 0)
			return (md5_err_usage());
	}
	while (parsed && parsed < argc)
	{
		opts.filename = argv[parsed];
		hash_file(argv[parsed], &opts);
		parsed++;
	}
	if (!parsed && !opts.p && !opts.q)
	{
		opts.input = (unsigned char *)get_from_fd(0, &(opts.input_size));
		md5(opts.input, opts.input_size, &opts);
		free(opts.input);
	}
	
	//md5(opts.str, opts.in_size);
	//if (!opts.p)
	//	free(opts.input);
	return (0);
	// return 1 in case of error
}
