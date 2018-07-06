# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: Ulliwy <Ulliwy@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2017/10/05 13:01:02 by iprokofy          #+#    #+#              #
#    Updated: 2018/07/05 21:09:44 by Ulliwy           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ft_ssl
SRC = main.c\
		b64/base64.c\
		buff_read.c\
		des/des_tables.c\
		des/des.c\
		print_bits.c\
		des/block_handle.c\
		put_err.c\
		des/init.c\
		des/key_gen.c\
		des/bit_permut.c\
		des/des_decode.c\
		des/des_encode.c\
		\
		md5_prep.c\
		md5_tables.c
FLAGS = -Wall -Wextra -Werror -O3 -flto -march=native

all: $(NAME)

libft/libft.a:
	$(MAKE) -C libft

ft_ssl: $(SRC) libft/libft.a
	clang $(FLAGS) $(SRC) -Llibft -lft -o $(NAME)

clean:
	rm -f $(SRC:.c=.o)
	rm -f $(LIBFT:.c=.o)
	rm -f -r ft_ssl.dSYM

fclean: clean
	rm -f $(NAME)
	$(MAKE) -C libft fclean

re: fclean all