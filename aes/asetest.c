/*
 * asetest.c
 *
 *  Created on: Oct 21, 2014
 *      Author: rainbow
 */

// Encryption.cpp : 定义控制台应用程序的入口点。
//
/*
 * main.c
 *
 *  Created on: Oct 20, 2014
 *      Author: rainbow
 */

#include "ase.h"

#include<stdio.h>
void printHex(byte* input, int nr)
{
	int i;

	for (i = 0; i < nr; i++)
	{
		int test = (int) input[i];
		printf("%.2x ", test);
	}
	putchar('\n');
}
int aestest()
{
	int i;
	unsigned char input[16] =
	{ 0x0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff };
	unsigned char key192[24] =
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
			21, 22, 23 };
	unsigned char key128[16] =
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	unsigned char keytest[16] =
	{ 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88,
			0x09, 0xcf, 0x4f, 0x3c };
	unsigned char *res = AES192(input, key192);
	for (i = 0; i < 16; i++)
	{
		printf("%.2x ", res[i]);
	}
	printf("\n");
	unsigned char *res2 = invAES192(res, key192);
	for (i = 0; i < 16; i++)
	{
		printf("%.2x ", res2[i]);
	}
	printf("\n");
	getchar();
	return 0;
}

