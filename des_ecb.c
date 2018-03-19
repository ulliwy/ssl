/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   des_ecb.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Ulliwy <Ulliwy@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/12/14 12:46:00 by iprokofy          #+#    #+#             */
/*   Updated: 2018/03/19 15:56:43 by Ulliwy           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ssl.h"
#include <stdio.h>

int		put_key_err()
{
	ft_putstr("non-hex digit\n");
	ft_putstr("invalid hex key value\n");
	return (0);
}

int get_keys3(t_opt *opts)
{
	int		i;
	int		k;
	char	c;

	i = 0;
	k = 0;
	//printf("%lu %lu %lu\n", opts->keys[0], opts->keys[1], opts->keys[2]);
	while (opts->entered_key[i] && opts->entered_key[i] != '\n' && i < 48)
	{
		c = ft_tolower(opts->entered_key[i]);
		if (!((c >= 'a' && c <= 'f') || (c >= '0' && c <= '9')))
			return (put_key_err());
		if (c >= 'a' && c <= 'f')
			c = c - 87;
		else
			c = c - '0';
		opts->keys[k] = opts->keys[k] * 16 + c;
		i++;
		if (i && (i % 16 == 0))
			k++;
	}
	// if (i && (i % 16 == 0))
	// 	k++;
	while (i < 48)
	{
		opts->keys[k] = opts->keys[k] * 16;
		i++;
		if (i && (i % 16 == 0))
			k++;
	}
	//printf("%lu %lu %lu\n", opts->keys[0], opts->keys[1], opts->keys[2]);
	return (1);
}

int		get_key(t_opt *opts)
{
	int		len;
	int		i;
	char	c;

	i = 0;
	len = ft_strlen(opts->entered_key);

	if (opts->entered_key[len - 1] == '\n')
		len--;
	while (i < len && i < 16)
	{
		c = ft_tolower(opts->entered_key[i]);
		if (!((c >= 'a' && c <= 'f') || (c >= '0' && c <= '9')))
			return (put_key_err());
		if (c >= 'a' && c <= 'f')
			c = c - 87;
		else
			c = c - '0';
		opts->main_key = opts->main_key * 16 + c;
		i++;
	}
	while (i < 16)
	{
		opts->main_key = opts->main_key * 16;
		i++;
	}
	//printf("%lX\n", opts->main_key);
	return (1);
}

unsigned long	permut_pc2(unsigned long c0, unsigned long d0)
{
	unsigned long	key;
	int				i;

	i = 0;
	key = 0;
	while (i < 48)
	{
		key = key | (((1UL << (56 - g_pc2[i])) & (c0 | d0)) ? 1UL << (47 - i) : 0);
		i++;
	}
	return (key);
}

void	permut_c0d0(unsigned long *c0, unsigned long *d0, unsigned long	keys[16])
{
	int i;
	unsigned long	cl;
	unsigned long	cr;
	unsigned long	dl;
	unsigned long	dr;

	i = 0;
	while (i < 16)
	{
		cr = *c0 >> (28 - g_shifts[i]);
		cl = *c0 << g_shifts[i];
		*c0 = (cl | cr) & ((1UL << 56) - (1UL << 28));
		dr = *d0 >> (28 - g_shifts[i]);
		dl = *d0 << g_shifts[i];
		*d0 = (dl | dr) & ((1UL << 28) - 1UL);
		keys[i] = permut_pc2(*c0, *d0);
		i++;
	}
}

void	create_subkeys(unsigned long k, unsigned long	keys[16])
{
	unsigned long	k_plus;
	unsigned long	c0;
	unsigned long	d0;
	int				i;

	i = 0;
	k_plus = 0;
	while (i < 56)
	{
		k_plus = k_plus | ((1UL << (64 - g_pc1[i])) & k ? 1UL << (55 - i) : 0);
		i++;
	}
	c0 = ((1UL << 56) - (1UL << 28)) & k_plus;
	d0 = ((1UL << 28) - 1UL) & k_plus;
	permut_c0d0(&c0, &d0, keys);
}

unsigned long	ff(unsigned long r, unsigned long k)
{
	unsigned long	er;
	unsigned long	result;
	int 			i;

	i = 0;
	er = 0;
	result = 0;
	while (i < 48)
	{
		er = er | ((1UL << (32 - g_ebit[i])) & r ? 1UL << (47 - i) : 0);
		i++;
	}
	er = er ^ k;
	i = 0;
	unsigned long p4 = 0;
	unsigned long f = 0;
	while (i < 8)
	{
		unsigned long p1 = (er >> ((7 - i) * 6)) & 63;
		unsigned long p2 = (1 & p1) | ((p1 >> 4) & 2);
		unsigned long p3 = (p1 >> 1) & 15;
		p4 = (p4 << 4) | (g_ss[i][p2 * 16 + p3]);
		i++;
	}
	int j = 0;
	while (j < 32)
	{
		f = (f << 1UL) | ((1UL << (32 - g_p[j])) & p4 ? 1UL : 0);
		j++;
	}
	return (f);
}

void	des_decryption(unsigned long **msg, int i, t_opt opts, unsigned long keys[16], ssize_t size, int fd)
{
	unsigned long	cur;
	unsigned long	temp;
	int j;
	unsigned long	l[17];
	unsigned long	r[17];

	cur = reverse_bits(*msg[i]);
	temp = 0;
	j = 0;
	while (j < 64)
	{
		temp = temp | ((cur & 1UL) << (64 - g_ip_1[63 - j]));
		cur = cur >> 1;
		j++;
	}
	
	r[16] = temp >> 32;
	l[16] = temp & ((1UL << 32) - 1UL);
	j = 16;
	while (j >= 1)
	{
		l[j - 1] = r[j] ^ ff(l[j], keys[j - 1]);
		r[j - 1] = l[j];
		j--;
	}
	l[0] = (l[0] << 32) | r[0];
	j = 0;
	temp = 0;
	while (j < 64)
	{
		temp = temp | ((l[0] & 1UL) << (64 - g_ip[63 - j]));
		l[0] = l[0] >> 1;
		j++;
	}
	if (!i && opts.cmd->cbc)
		temp = temp ^ opts.v;
	if (i > 0 && opts.cmd->cbc)
		temp = temp ^ reverse_bits(*msg[i - 1]);
	int val = temp & 255;
	temp = reverse_bits(temp);
	*msg[i] = temp;
	printf("%lu\n", size);
	printf("%d\n", val);
	write(fd, &temp, sizeof(temp) - (i == size / 8 - 1 ? val : 0) + 1);
}

unsigned long	des_dencryption(unsigned long **msg, int i, t_opt opts, unsigned long keys[16])
{
	unsigned long	temp;
	unsigned long	cur;
	int 			j;
	unsigned long	l[17];
	unsigned long	r[17];

	cur = reverse_bits((*msg)[i]);
	temp = 0;
	j = 0;
	while (j < 64)
	{
		temp = temp | ((cur & 1UL) << (64 - g_ip_1[63 - j]));
		cur = cur >> 1;
		j++;
	}
	
	r[16] = temp >> 32;
	l[16] = temp & ((1UL << 32) - 1UL);
	j = 16;
	while (j >= 1)
	{
		l[j - 1] = r[j] ^ ff(l[j], keys[j - 1]);
		r[j - 1] = l[j];
		j--;
	}
	l[0] = (l[0] << 32) | r[0];
	j = 0;
	temp = 0;
	while (j < 64)
	{
		temp = temp | ((l[0] & 1UL) << (64 - g_ip[63 - j]));
		l[0] = l[0] >> 1;
		j++;
	}
	if (!i && opts.cmd->cbc)
		temp = temp ^ opts.v;
	if (i > 0 && opts.cmd->cbc)
		temp = temp ^ reverse_bits((*msg)[i - 1]);
	//printf("val: %d\n", val);
	//temp = reverse_bits(temp);
	//(*msg)[i] = temp;
	//write(fd, &temp, sizeof(temp) - (i == size / 8 - 1 ? val : 0));
	return (temp);
}

void	des_ecb_decode(unsigned char *in, int fd, ssize_t size, t_opt opts)
{
	unsigned long	keys[16];
	unsigned long	*msg;
	int 			i;
	unsigned long temp;

	create_subkeys(opts.main_key, keys);
	if (opts.a)
		msg = (unsigned long *)b64_decode(in, &size);
	else
		msg = (unsigned long *)in;
	i = 0;

	while (i < size / 8)
	{
		temp = des_dencryption(&msg, i, opts, keys);
		int val = temp & 255;
		temp = reverse_bits(temp);
		write(fd, &temp, sizeof(temp) - (i == size / 8 - 1 ? val : 0));
		i++;
	}
	if (opts.a)
		free(msg);
}

unsigned long	des_encryption(unsigned long **msg, int i, t_opt opts, unsigned long keys[16])
{
	unsigned long	temp;
	int 			j;
	unsigned long	l[17];
	unsigned long	r[17];

	(*msg)[i] = reverse_bits((*msg)[i]);
	if (!i && opts.cmd->cbc)
		(*msg)[0] = (*msg)[0] ^ opts.v; 
	if (i > 0 && opts.cmd->cbc)
		(*msg)[i] = reverse_bits((*msg)[i - 1]) ^ (*msg)[i];
	temp = 0;
	j = 0;
	//initial permutation
	while (j < 64)
	{
		temp = temp | (((1UL << (64 - g_ip[j])) & (*msg)[i]) ? 1UL << (63 - j) : 0);
		j++;
	}
	l[0] = temp >> 32;
	r[0] = temp & ((1UL << 32) - 1UL);
	j = 1;
	while (j <= 16)
	{
		l[j] = r[j - 1];
		r[j] = l[j - 1] ^ ff(r[j - 1], keys[j - 1]);
		j++;
		
	}

	r[16] = (r[16] << 32) | l[16];
	j = 0;
	temp = 0;
	while (j < 64)
	{
		temp = temp | (((1UL << (64 - g_ip_1[j])) & r[16]) ? 1UL << (63 - j) : 0);
		j++;
	}
	(*msg)[i] = reverse_bits(temp);
	return ((*msg)[i]);
}

void	des_ecb_encode(unsigned char *in, int fd, ssize_t size, t_opt opts)
{
	unsigned long	keys[3][16];
	unsigned long	*msg;
	unsigned long	temp;
	int 			i;
	// int 			j;
	// unsigned long	l[17];
	// unsigned long	r[17];

	pad(in, size);
	if (opts.cmd->ecb3 || opts.cmd->cbc3)
	{
		create_subkeys(opts.keys[0], keys[0]);
		create_subkeys(opts.keys[1], keys[1]);
		create_subkeys(opts.keys[2], keys[2]);
	} else
		create_subkeys(opts.main_key, keys[0]);
	msg = (unsigned long *)in;
	i = 0;
	while (i < size / 8 + 1)
	{
		des_encryption(&msg, i, opts, keys[0]);
		if (opts.cmd->ecb3 || opts.cmd->cbc3)
		{
			temp = des_dencryption(&msg, i, opts, keys[1]);
			temp = reverse_bits(temp);
			msg[i] = temp;
			msg[i] = des_encryption(&msg, i, opts, keys[2]);
		}
		i++;
	}
	in = (unsigned char *)msg;
	opts.in = in;
	if (opts.a)
		b64_encode(&opts, (size / 8 + 1) * sizeof(unsigned long));
	else
		write(fd, msg, (size / 8 + 1) * sizeof(unsigned long));
}

void	des_ecb(t_opt *opts)
{
	char	*input;
	ssize_t r;

	if (!opts->input_file)
		input = get_from_fd(0, &r);
	else
	{
		if ((opts->fd = open(opts->input_file, O_RDONLY)) == -1)
		{
			put_open_err(opts->input_file);
			return ;
		}
	 	input = get_from_fd(opts->fd, &r);
	 	close(opts->fd);
	}
	if (!opts->output_file) {
		opts->fd = 1;
	}
	else if ((opts->fd = open(opts->output_file, O_CREAT | O_WRONLY | O_APPEND | O_TRUNC, 0644)) == -1)
	{
		put_open_err(opts->output_file);
		return ;
	}
	
	if (opts->cmd->cbc3)
		opts->cmd->cbc = 1;
	if (opts->d)
		des_ecb_decode((unsigned char *)input, opts->fd, r, *opts);
	else
		des_ecb_encode((unsigned char *)input, opts->fd, r, *opts);
	close(opts->fd);
	free(input);
}

void	des_prep(t_opt opts)
{
	int		allocated;

	allocated = 0;
	if (!opts.entered_key)
	{
		opts.entered_key = (char *)ft_memalloc(100);
		ft_putstr("enter des key in hex: ");
		read(1, opts.entered_key, 100);
		allocated = 1;
	}
	if (opts.cmd->ecb3 || opts.cmd->cbc3)
	{
		if (!get_keys3(&opts))
			return ;
	}
	else if(!get_key(&opts))
		return ; 
	des_ecb(&opts);
	if (allocated)
		free(opts.entered_key);
}
