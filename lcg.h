// SPDX-License-Identifier: MIT
/*  
 *  lcg.h - Logistic Chaos bit Generator definitions and declarations.
 *
 *  Copyright 2019 Hideto Manjo.
 *  Written by:  Hideto Manjo <enjinit@gmail.com>
 *
 */
#ifndef _LCG_H
#define _LCG_H

#include <stdio.h>

#define AI			   4  /*initial a, AI=4 means complete chaos*/
#define BLOCK_SIZE		   2  /*default block size (bytes)*/

#define USE_DBL			   1  /*double or float*/

#define BCD(c)	(5 * (5 * (5 * (5 * (5 * (5 * (5 * (c & 128) + (c & 64)) \
	+ (c & 32)) + (c & 16)) + (c & 8)) + (c & 4)) + (c & 2)) + (c & 1))

#if USE_DBL
#define X_FLOAT_TYPE          double  /*x precision*/
#define BITWISE            bitwise64  /*union bitwise*/
#else
#define X_FLOAT_TYPE           float 
#define BITWISE            bitwise32
#endif

struct double_inner {
	unsigned long int coefficient : 52;
	unsigned int radix : 11;
	unsigned int sign : 1;
};

struct float_inner {
	unsigned int coefficient : 23;
	unsigned int radix : 8;
	unsigned int sign : 1;
};

/**
 * union bitwise64 - View of 64bit float point number
 */
union bitwise64 {
	double f;
	long long int i;
	unsigned char c[8];
	struct double_inner inner;		
};

/**
 * union bitwise64 - View of 32bit float point number
 */
union bitwise32 {
	float f;
	long int i;
	unsigned char c[4];
	struct float_inner inner;
};

/**
 * struct block_encode_result - encode result
 */
struct lcg_operation_result {
	unsigned long long int count;		/*total retry count*/
	unsigned long int block_count;		/*block count*/
	size_t block_size;			/*block size*/
	unsigned long int write_bytes;		/*write_bytes*/
	unsigned long int read_bytes;		/*read_bytes*/
};

extern void lcg_init_seed(void);
extern int lcg_encode(FILE *fp_in, FILE *fp_out, const size_t block_size,
		      struct lcg_operation_result *result);
extern int lcg_decode(FILE *fp_in, FILE *fp_out, const size_t block_size,
	       	      struct lcg_operation_result *result);
extern int lcg_split_xor(FILE *fp_in, FILE *fp_out_key, FILE *fp_out_bin);
extern int lcg_join_xor(FILE *fp_in, FILE *fp_in_key, FILE *fp_out_bin);
#endif		/* _LCG_H */