/*
 * rsa.h
 *
 *  Created on: Oct 21, 2014
 *      Author: rainbow
 */

#ifndef RSA_H_
#define RSA_H_

#include"../largenum/lnum.h"
#include"../largenum/lnum_algo/lnum_algo.h"

typedef struct _rsa_key
{
	large_num e_or_d; //一般为65537
	large_num n;
	enum {PRIVATE,PUBLIC}type;
}rsa_key;

int rsa_get_key(unsigned int bit, rsa_key *pri, rsa_key *pub);
int rsa_encode(char content[], char res[], int len, rsa_key key);
int rsa_decode(char content[], char res[], int len, rsa_key key);
#endif /* RSA_H_ */
