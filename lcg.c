// SPDX-License-Identifier: MIT
/*  
 *  lcg.c - Logistic Chaos bit Generator
 *
 *  This program encode the binary data to its logistic map
 *  origin with initial value x0.
 *
 *  Copyright 2019 Hideto Manjo.
 *  Written by:  Hideto Manjo <enjinit@gmail.com>
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include "lcg.h"

static unsigned long long int lcg_seed = 88172645463325252LL;

/**
 * dump_bin() - Print bin and hex.
 *
 * @title:       message title
 * @byte_array:  char array
 * @size:        the length of print
 */
void dump_bin(char* title, unsigned char* byte_array,
	      const unsigned int size)
{
	int i;

	printf("%8s bin: ", title);
	
	for (i = 0; i < size; i++) 
		printf("%08d ", BCD(byte_array[i]));

	printf("\n%8s hex: ", title);
	
	for (i = 0; i < size; i++)
		printf("0x%02X ", byte_array[i] & 0x000000FF);	
	
	printf("\n");
}

/**
 * cmp_bytes - Comparison of byte arrays.
 *
 * @array1:  char array
 * @array2:  char array
 * @size:    the length of compare
 */
static int cmp_bytes(unsigned char* array1, unsigned char* array2,
		     unsigned int size)
{
	int i;

	for (i = 0; i < size; i++) {
		if (*(array1 + i) != *(array2 + i))
			return 0;
	}
	return 1;
}

/**
 * cp_bytes - Copy of byte arrays.
 * 
 * @array1:   origin array
 * @array2:   target array
 * @size:     copy length
 */
static void cp_bytes(unsigned char* array1, unsigned char* array2,
	      unsigned int size)
{
	int i;

	for (i = 0; i < size; i++) {
		*(array2 + i) = *(array1 + i);
	}
}

/**
 * lcg_init_seed() - Initilize seed of xor64(). 
 */
void lcg_init_seed(void)
{
	srandom((unsigned int) time(NULL));
	lcg_seed ^= (unsigned long long int) random();
	lcg_seed = lcg_seed << 23;
	lcg_seed ^= (unsigned long long int) random();
}

/**
 * xor64() - Create 64bit pseudorandom numbers using xorshift method
 * 
 * Period of pseudo random number is 2^64 -1.
 *
 */
static unsigned long long xor64(void)
{
	unsigned long long *x = &lcg_seed;

	*x ^= (*x << 13);
	*x ^= (*x >> 7);
	return (*x ^= (*x << 17));
}

/**
 * uniform_rand() - Generate a uniform random number normalized from 0 to 1.
 */
static X_FLOAT_TYPE uniform_rand(void)
{	
	return (
		((X_FLOAT_TYPE) xor64() + 1.0)
		/ ((X_FLOAT_TYPE) ULLONG_MAX + 2.0)
	);
}

/**
 * logistic() - Return logistic map.
 */
static X_FLOAT_TYPE logistic(const X_FLOAT_TYPE *x)
{       
	return AI * (*x) * (1 - (*x));
}

/**
 * degitize() - Digitize to 1bit.
 * 
 * This function return unsigned char(0 or 1).
 * 
 */
static unsigned char degitize(const X_FLOAT_TYPE *x)
{
	if (*x < 0.5)
		return 0;
	return 1;
}

/**
 * eight_bit() - Write 8 bits from the initial value of the logistic map.
 * 
 * @x0:             initial x
 * @byte_array      output array
 *
 * This function uses digitize().
 *
 */
static void eight_bit(X_FLOAT_TYPE *x0, unsigned char *byte_array)
{
	int i = CHAR_BIT; /*1byte = 8bit*/

	while(i--) {
		*x0 = logistic(x0);
		*byte_array = *byte_array << 1;
		*byte_array = *byte_array | degitize(x0);
	}
}

/**
 * eight_bit_r() - eight_bit() with check up bit.
 * 
 * @x0           initial x
 * @byte_array   output array
 * @target_bin   target array
 *
 * Returns false imidiately when generated bits is not target binary.
 * 
 * Returns:
 *  1: success
 *  0: fail
 */
static int eight_bit_r(X_FLOAT_TYPE *x0, unsigned char *byte_array,
		const unsigned char *target_bin)
{
	int i = CHAR_BIT;
	X_FLOAT_TYPE x = *x0;
	unsigned char d;
	*byte_array = 0;

	while(i--) {
		x = logistic(&x);
		d = degitize(&x);
		
		if (d != ((*target_bin >> i) & 1))
			return 0;
		*byte_array = *byte_array << 1;
		*byte_array = *byte_array | d;
	}

	*x0 = x;	
	return 1;
}

/**
 * read_block() - Read bin array from file.
 * @fp:    file pointer
 * @bin:   binary array
 * @size:  block size
 *
 * Returns:
 *  >=1: success(read size)
 *  0: fail
 */
static int read_block(FILE *fp, unsigned char* bin, unsigned int size)
{	
	unsigned int l;
	if ((l = fread(bin, sizeof(unsigned char), size, fp)) < 1)
		return 0;
	return l;
}

/**
 * write_block() - Write bin array to file.
 * @fp:    file pointer
 * @bin:   binary array
 * @size:  block size
 *
 * Returns:
 *  >=1: success(wrote size)
 *  0:  fail
 */
static int write_block(FILE *fp, unsigned char* bin, unsigned int size)
{	
	unsigned int l;
	if ((l = fwrite(bin, sizeof(unsigned char), size, fp)) < 1)
		return 0;
	return l;
}

/**
 * encode_block() - Encode block.
 *
 * @counter:    number of logistic map calls   
 * @x0:         initial x
 * @bin:        target array 
 * @byte_array: output array
 * @size:       block size(bytes)
 *
 * Returns:
 *  1: success
 *  0: yet 
*/
static int encode_block(unsigned long long int *counter, X_FLOAT_TYPE *x0,
		 unsigned char *bin, unsigned char *byte_array,
		 const int size)
{

	int j = 0;
	int max_j = 0;;
	X_FLOAT_TYPE x;

	/*x0*/
	*x0 = uniform_rand();
	x = *x0;

	while (1) {
		
		*counter = *counter + 1;
		
		if (!eight_bit_r(&x, &byte_array[j], &bin[j])) {
			j = 0;
			*x0 = uniform_rand();
			x = *x0;
			continue;
		}

		if (j > max_j)
			max_j = j;

		if (*counter % 1000 == 0) {
			/*change random seed by random timing*/
			lcg_init_seed();
			printf("\rtry %19llu bytes, match: %4d", 
			       *counter, max_j + 1);
		}

		if (
			j == size - 1
			&& cmp_bytes(&bin[0], &byte_array[0], size)
		) {	
			printf("\rhit %19llu bytes, match: %4d\n", 
			       *counter, max_j + 1);	
			return 1;
		}

		j++;
	}
}

/**
 * encode() - Read binary file and encode it.
 * @fp_in:   read file.
 * @fp_out:  out file.
 *
 * Return:
 *  1: success
 *  0: yet
 */
int lcg_encode(FILE *fp_in, FILE *fp_out, const unsigned int block_size,
	       struct lcg_operation_result *result)
{	
	const int sizeof_x = sizeof(X_FLOAT_TYPE);

	/*target binary*/
	unsigned char bin[block_size];
	/*generated binary*/                 
	unsigned char byte_array[block_size];
	/*union for X_FLOAT_TYPE to char*/
	union BITWISE bitview;

	X_FLOAT_TYPE x0;

	unsigned long long int i;
	unsigned long block_count = 0;

	unsigned int read_length;
	unsigned int last_length = block_size;
	
	result->count = 0;

	while((read_length = read_block(fp_in, &bin[0], block_size))) {
		i = 0;
		encode_block(&i, &x0, &bin[0],  &byte_array[0], block_size);
	
		bitview.f = x0;	
		if (read_length < block_size) {
			bitview.c[sizeof_x - 1] = 0x40 + read_length;
			last_length = read_length;
		}
		write_block(fp_out, &bitview.c[0], sizeof_x);
		result->count += i;
		block_count++;
	}

	result->block_size = block_size;
	result->block_count = block_count;
	result->read_bytes = (block_count - 1) * block_size + last_length;
	result->write_bytes = block_count * sizeof_x;
	
	return 1;
}

/**
 * decode() - Read binary file and decode it.
 *
 * @fp_in:   readfile
 * @fp_out:  outfile 
 *
 * Return:
 *  1: success
 *  0: fail
 */
int lcg_decode(FILE *fp_in, FILE *fp_out, const unsigned int block_size,
	       struct lcg_operation_result *result)
{	
	const int sizeof_x = sizeof(X_FLOAT_TYPE); 

	int j;

	/*union for X_FLOAT_TYPE to char*/
	union BITWISE bitview;
	/*read binary*/
	unsigned char bin[sizeof_x];
	/*generated binary*/
	unsigned char byte_array[block_size];

	X_FLOAT_TYPE *x0;

	unsigned long int block_count = 0;
	unsigned int write_block_size = block_size;
	unsigned int read_length;

	while((read_length = read_block(fp_in, &bin[0], sizeof_x))) {
		if (read_length != sizeof_x)
			return 0;

		/*If end block, fix block size and add bin 0x3F*/
		if (bin[sizeof_x - 1] > 0x3F) {
			write_block_size = bin[sizeof_x - 1] - 0x40;
			bin[sizeof_x - 1] = 0x3F;
		}

		cp_bytes(bin, bitview.c, sizeof_x);
		x0 = &bitview.f;
		
		for (j =0; j < block_size; j++) {
			eight_bit(x0, &byte_array[j]);
		}

		write_block(fp_out, &byte_array[0], write_block_size);
		block_count++;
	}

	result->count = 0;
	result->block_count = block_count;
	result->read_bytes = block_count * sizeof_x;
	result->write_bytes = (block_count - 1) * block_size + write_block_size;

	return 1;
}


/**
 * lcg_xor() - XOR operation by 1byte.
 *
 * @x:		input x char array
 * @y:		input y char array
 * @out: 	output char array
 */
void lcg_xor(unsigned char *x, unsigned char *y,
	     unsigned char *out, const unsigned int size)
{	
	int i = size;
	while(i--)
		*(out + i) = *(x + i) ^ *(y + i);
}

/**
 * lcg_split_xor() - split file sub using random.
 */
int lcg_split_xor(FILE *fp_in, FILE *fp_out_key, FILE *fp_out_bin)
{	

	union bitwise64 bitview;
	unsigned char bin[8];
	unsigned char xor_out[8];
	unsigned char random_point = 0x00;
	int write_block_size;

	lcg_init_seed();

	while((write_block_size = read_block(fp_in, &bin[0], 8)) > 0) {
		bitview.i = xor64();

		/*random seed change*/
		if (bitview.c[7] == random_point) {
			lcg_init_seed();
			random_point = bitview.c[3] ^ bin[5];
		}

		lcg_xor(&bin[0], &bitview.c[0], &xor_out[0], write_block_size);

		write_block(fp_out_key, &bitview.c[0], write_block_size);
		write_block(fp_out_bin, &xor_out[0], write_block_size);
	}

	return 1;
}

/**
 * lcg_join_xor() - join file xor key file.
 */
int lcg_join_xor(FILE *fp_in, FILE *fp_in_key, FILE *fp_out_bin)
{	
	unsigned char bin[8];
	unsigned char key[8];
	unsigned char out[8];
	int write_block_size;

	while(1) {
		write_block_size = read_block(fp_in, &bin[0], 8);
		if (write_block_size != read_block(fp_in_key, &key[0], 8))
			return 0;

		if (write_block_size < 1)
			break;

		lcg_xor(&bin[0], &key[0], &out[0], write_block_size);
		write_block(fp_out_bin, &out[0], write_block_size);
	}

	return 1;
}