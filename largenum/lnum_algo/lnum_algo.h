/*
 * lnum_algo.h
 *
 *  Created on: Nov 7, 2014
 *      Author: rainbow
 */

#ifndef LNUM_ALGO_H_
#define LNUM_ALGO_H_

#include"../lnum.h"
#include<time.h>
#include<ctype.h>

large_num lnum_gcd(large_num a, large_num b);
large_num lnum_gcd_ex(large_num a, large_num b, large_num *x, large_num *y);
large_num lnum_random_by_bit(unsigned int nr_bit);
void lnum_print(large_num num,char* name);
int lnum_test_prime_simple(large_num n);
int lnum_test_prime_miller_rabin(large_num n, size_t s);
large_num lnum_mod_exp(large_num e, large_num x, large_num n);
large_num lnum_get_prime_by_bit(int bit_nr, int try_count);
size_t lnum_get_bit_count(large_num x);
large_num lnum_from_string(const char *str);
#endif /* LNUM_ALGO_H_ */
