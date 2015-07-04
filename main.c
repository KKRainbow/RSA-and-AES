// Encryption.cpp : 定义控制台应用程序的入口点。
//
/*
 * main.c
 *
 *  Created on: Oct 20, 2014
 *      Author: rainbow
 */

#include<stdio.h>

#include "largenum/lnum.h"
#include"largenum/lnum_algo/lnum_algo.h"
#include"rsa/rsa.h"
#include <math.h>
#include<string.h>
unsigned long get_file_size(FILE *fp)
{
	unsigned long filesize = -1;
	if (fp == NULL)
		return filesize;
	fseek(fp, 0L, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return filesize;
}
int main(int argc, char **argv)
{
	if (argc == 3 && strcmp(argv[1], "gen") == 0) //位数
	{
		if (argc < 3)
			printf("错误，参数太少！\n");
		rsa_key pub, pri;
		rsa_get_key(atoi(argv[2]), &pri, &pub);

		printf("pub:\n");
		lnum_print(pub.n, "key");
		lnum_print(pub.e_or_d, "e");

		printf("\npri:\n");
		lnum_print(pri.n, "key");
		lnum_print(pri.e_or_d, "d");
		putchar('\n');
	}
	else if (argc == 5 && strcmp(argv[1], "de") == 0) //解密 先n 后e 最后时密文文件
	{
		printf("%s\n",argv[4]);
		FILE* fp = fopen(argv[4], "rb");
		unsigned int fsize = get_file_size(fp);
		rsa_key pub;
		pub.n = lnum_from_string(argv[2]);
		pub.e_or_d = lnum_from_string(argv[3]);
		pub.type = PUBLIC;
		lnum_print(pub.n, "pub.n");
		lnum_print(pub.e_or_d, "pub.e");

		char* encode = calloc(fsize * 100, sizeof(char));
		char* decode = calloc(fsize * 100, sizeof(char));

		fread(encode, fsize, 1, fp);

		rsa_decode(encode, decode, strlen(encode), pub);

		printf("解密后:%s\n",decode);
		fclose(fp);
	}
	else if (argc == 6 && strcmp(argv[1], "en") == 0) //解密 先n 后e 密文 最后时密文文件
	{
		FILE* fp = fopen(argv[5], "wb");
		rsa_key pub;

		pub.n = lnum_from_string(argv[2]);
		pub.e_or_d = lnum_from_string(argv[3]);
		lnum_print(pub.n, "pub.n");
		lnum_print(pub.e_or_d, "pub.e");
		pub.type = PUBLIC;

		char* encode = calloc(strlen(argv[4]) * 100, sizeof(char));
		char* decode = calloc(strlen(argv[4]) * 100, sizeof(char));
		strcpy(decode,argv[4]);
		rsa_encode(decode, encode, strlen(argv[4]), pub);
		fwrite(encode, strlen(argv[4]) * 100, 1, fp);
		fclose(fp);
	}
	else
	{
		printf("Usage:\n");
		printf("encryption gen 密码长度（位数）\n");
		printf("encryption en key.n key.e your_str file_to_save\n");
		printf("encryption de key.n key.d file_to_read\n");
	}
}
