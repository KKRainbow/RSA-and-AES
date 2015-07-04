/*
* ase.h
*
*  Created on: Oct 20, 2014
*      Author: rainbow
*/

#ifndef ASEF_ASE_H_
#define ASEF_ASE_H_

#include<malloc.h>
#include<string.h>
typedef unsigned char byte;
typedef struct{ byte bit[4]; } word;

byte* AES128(byte* input, byte* key);
byte* AES192(byte* input, byte* key);
byte* AES256(byte* input, byte* key);
byte* invAES128(byte* input, byte* key);
byte* invAES192(byte* input, byte* key);
byte* invAES256(byte* input, byte* key);
#endif /* ASEF_ASE_H_ */
