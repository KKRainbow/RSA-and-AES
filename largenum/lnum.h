/*
 * lnum.h
 *
 *  Created on: Nov 3, 2014
 *      Author: rainbow
 */

#ifndef LNUM_H_
#define LNUM_H_
#include<stdlib.h>
#include<limits.h>
#include<string.h>
#include<stdio.h>

#define _WHICH_BIT_SIZE 4

#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<=b?a:b)
#define _LNUM_FREE(des) if((des)->bits){free((des)->bits);(des)->bits = NULL;(des)->length=0;}
#define _LNUM_CALLOC(bit_length) (bit_t*)calloc(bit_length,sizeof(bit_t))
#define _LNUM_SHRINK(tmp) {\
	for(int i = tmp.length -1;i>=0;i--)\
	{\
		if(tmp.bits[i] != 0)\
		{\
			tmp.length = i + 1;\
			break;\
		}\
	}\
	}


#define DEFAULE_BIT 128
#define SIGN_PLUS 0
#define SIGN_MINUS 1

#if _WHICH_BIT_SIZE == 4

typedef unsigned int bit_t;
#define _BIT_MAX UINT_MAX
typedef unsigned long long mid_t; //作为乘法的中间值 这个很重要
#define _FORMAT_CHAR "%.8x"

#elif _WHICH_BIT_SIZE == 1

typedef unsigned char bit_t;
typedef unsigned short mid_t; //作为乘法的中间值 这个很重要
#define _FORMAT_CHAR "%.2x"
#define _BIT_MAX UCHAR_MAX

#elif _WHICH_BIT_SIZE == 2

typedef unsigned short bit_t;
typedef unsigned int mid_t; //作为乘法的中间值 这个很重要
#define _FORMAT_CHAR "%.4x"
#define _BIT_MAX USHRT_MAX

#elif _WHICH_BIT_SIZE == 10

typedef unsigned char bit_t;
typedef unsigned short mid_t; //作为乘法的中间值 这个很重要
#define _BIT_SIZE 0.5
#define _BIT_MAX 9

#else

typedef unsigned int bit_t;
#define _BIT_MAX UINT_MAX
typedef unsigned long long mid_t; //作为乘法的中间值 这个很重要
#define _FORMAT_CHAR "%.8x"
#define _BIT_SIZE 4
#pragma message("no bit size chosen,default:4.please choose a bit size from 1 2 4")

#endif

#ifndef _BIT_SIZE
#define _BIT_SIZE _WHICH_BIT_SIZE
#endif

#ifndef _FORMAT_CHAR
#define _FORMAT_CHAR "%x"
#endif

typedef char sign_t;

typedef struct _large_num{
	bit_t* bits;
	size_t length;
	sign_t sign;
}large_num;

#define E_MEM 1
#define E_PROG 2 //库本身有问题
#define E_DZERO 3//除0错误


int lnum_cmp(large_num a, large_num b);
int lnum_iszero(large_num obj);
void lnum_mov(large_num *des, large_num *src);

void lnum_add(large_num a, large_num b,large_num* des);
void lnum_sub(large_num a, large_num b,large_num* des);
//暂时采用N^2的算法
void lnum_mul(large_num a, large_num b,large_num* des );
void lnum_div(large_num* rem, large_num a, large_num b,large_num* quo );
void lnum_pow(large_num e, large_num x,large_num* des );

large_num lnum_add_(large_num a, large_num b );
large_num lnum_sub_(large_num a, large_num b);
//暂时采用N^2的算法
large_num lnum_mul_(large_num a, large_num b);
large_num lnum_div_(large_num* rem, large_num a, large_num b);
large_num lnum_pow_(large_num e, large_num x);
large_num lnum_rem_(large_num a, large_num b);
large_num lnum_quo_(large_num a, large_num b);

void lnum_init1(large_num* des);
void lnum_init2(large_num* des);
void lnum_init0(large_num* des);
void lnum_initx(large_num* des,unsigned long long x,sign_t sign);

void lnum_tostring(char* buffer,large_num* des);
int lnum_errno();
#endif /* LNUM_H_ */
