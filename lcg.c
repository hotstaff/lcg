// SPDX-License-Identifier: MIT
/*  
 *  lcg.c - Logistic Chaos Generator sample program
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
#include <math.h>
#include <getopt.h>

#define AI 	   	           4  /*initial a, AI=4 means complete chaos*/
#define LENGTH 		           2  /*default block length (bytes)*/

#define FILENAME 	           "sample.bin"
#define OUTPUT                     "output.bin"

#define BCD(c)	5 * (5 * (5 * (5 * (5 * (5 * (5 * (c & 128) + (c & 64)) \
	+ (c & 32)) + (c & 16)) + (c & 8)) + (c & 4)) + (c & 2)) + (c & 1)



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

union bitwise64 {
	double d;
	long int l;
	unsigned char c[8];
	struct double_inner inner;		
};

union bitwise32 {
	float f;
	int i;
	unsigned char c[4];
	struct float_inner inner;
};




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
	int i = size;
	
	printf("%8s bin: ", title);
	while(i--) {
		printf("%08d", BCD(byte_array[i]));
	}
	printf("\n");

	i = size;
	printf("%8s hex: ", title);
	while(i--) {
		printf("0x%02X ", byte_array[i] & 0x000000FF);	
	}
	printf("\n");
}


/**
 * cmp_bytes - Comparison of byte arrays.
 *
 * @array1:  char array
 * @array2:  char array
 * @size:    the length of compare
 */
int cmp_bytes(unsigned char* array1, unsigned char* array2,
	      unsigned int size)
{
	int i;

	for (i = 0; i < size; i++) {
		if (*(array1 + i) != *(array2 + i)) {
			return 0;
		}
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
void cp_bytes(unsigned char* array1, unsigned char* array2,
	      unsigned int size)
{
	int i;

	for (i = 0; i < size; i++) {
		*(array2 + i) = *(array1 + i);
	}
}


/**
 * uniform_rand() - Generate a uniform random number normalized from 0 to 1.
 */
double uniform_rand(void)
{
	return ((double)rand()+1.0)/((double)RAND_MAX+2.0);
}


/**
 * logistic() - Return logistic map.
 */
double logistic(const double *x)
{       
	return AI * (*x) * (1 - (*x));
}


/**
 * degitize() - Digitize to 1bit.
 * 
 * This function return unsigned char(0 or 1).
 * 
 */
unsigned char degitize(const double *x)
{
	if (*x < 0.5) {
		return 0;
	}
	return 1;
}


/**
 * degitize_char() - Digitize to 8bit.
 *
 * Cast unsigned int(0 - 255).
 */
unsigned char digitize_char(const double *x)
{	
	return (unsigned char)((UCHAR_MAX + 1.0) * (*x));

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
void eight_bit(double *x0, unsigned char *byte_array)
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
int eight_bit_r(double *x0, unsigned char *byte_array,
		const unsigned char *target_bin)
{
	int i = CHAR_BIT;
	double x = *x0;
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
 * one_byte() - Write 8 bits from the initial value of the logistic map.
 *
 * @x0:          initial x
 * @byte_array:  output array
 *
 * This function uses digitize_char().
 */
void one_byte(double *x0, unsigned char *byte_array)
{	
	*x0 = logistic(x0);
	*byte_array = digitize_char(x0);
}


/**
 * read_block() - Read bin array from file.
 * @fp:    file pointer
 * @bin:   binary array
 * @size:  block size
 *
 * Returns:
 *  1: success
 *  0: fail
 */
int read_block(FILE *fp, unsigned char* bin, unsigned int size)
{	
	if (fread(bin, sizeof(unsigned char), size, fp) < 1) {
		return 0;
	}
	return 1;
}

/**
 * write_block() - Write bin array to file.
 * @fp:    file pointer
 * @bin:   binary array
 * @size:  block size
 *
 * Returns:
 *  1: success
 *  0: fail
 */
int write_block(FILE *fp, unsigned char* bin, unsigned int size)
{	
	if (fwrite(bin, sizeof(unsigned char), size, fp) < 1) {
		return 0;
	}
	return 1;
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
 *  0: success
 *  1: yet 
*/
int encode_block(long int *counter, double *x0,
		 unsigned char *bin, unsigned char *byte_array,
		 const int size)
{

	int j = 0;
	int max_j = 0;;
	double x;

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

		if (j > max_j) {
			max_j = j;
		}

		//if (*counter % 100 == 0)
		//	printf("\rtry %17ld bytes, match: %4d", 
		//	       *counter, max_j + 1);

		if (
			j == size - 1
			&& cmp_bytes(&bin[0], &byte_array[0], size)
		) {	
		//	printf("\rhit %17ld bytes, match: %4d\n", 
		//	       *counter, max_j + 1);	
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
 *  0: yet.
 */
int encode(FILE *fp_in, FILE *fp_out, const unsigned int block_size)
{	
	/*target binary*/
	unsigned char bin[block_size];            /*target binary*/
	/*generated binary*/                 
	unsigned char byte_array[block_size];
	/*union for double to char*/
	union bitwise64 bitview;

	double x0;
	long int i;
	int block_no = 0;

	/*random seed init*/
	srand((unsigned)time(NULL));

	while(read_block(fp_in, &bin[0], block_size)) {
		i = 0;
		encode_block(&i, &x0, &bin[0],  &byte_array[0], block_size);
		bitview.d = x0;
		// dump_bin("gen", &byte_array[0], block_size);
		// printf("Block %d x0: %.16f\n", block_no, x0);
		// dump_bin("x0", &bitview.c[0], 8);
		write_block(fp_out, &bitview.c[0], 8);
		block_no++;
	}

	printf("Encode %dblocks(%dbytes).\n",
	       block_no,
	       (block_no) * 8);
	
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
 *  0: yet
 */
int decode(FILE *fp_in, FILE *fp_out, const unsigned int block_size)
{	
	/*readed binary*/
	unsigned char bin[8];
	/*generated binary*/
	unsigned char byte_array[block_size];
	/*union for double to char*/
	union bitwise64 bitview;

	double *x0;
	int block_no = 0;
	int j;

	while(read_block(fp_in, &bin[0], 8)) {
		cp_bytes(bin, bitview.c, 8);
		x0 = &bitview.d;
		
		for (j =0; j < block_size; j++) {
			eight_bit(x0, &byte_array[j]);
		}

		write_block(fp_out, &byte_array[0], block_size);
		block_no++;
	}

	printf("Decode %dblocks(%dbytes).\n",
	       block_no,
	       (block_no) * block_size);

	return 1;
}


/**
 * request_fp_open() - Request file open.
 *
 * @fp:        file pointer
 * @filename:  filename
 * @mode:      mode
 *
 * Print the error message to stderr when the program can not open file.
 * 
 * Returns:
 *  FILE*: success
 *  NULL:  fail
 */
FILE *request_fp_open(FILE *fp, const char *filename, const char *mode)
{
	if ((fp = fopen(filename, mode)) == NULL) {
		fprintf(stderr, "File open error: %s.\n", filename);
		return fp;
	}
	return fp;
}

int main (int argc, char **argv)
{       
	FILE *fp_in;
	FILE *fp_out;

	int opt;

	int opt_decode = 0;
	unsigned int block_size = LENGTH;

	while ((opt = getopt(argc, argv, "ds:")) != -1) {
    		switch (opt) {
    			case 'd':
    				opt_decode = 1;
    				break;

    			case 's':
    				block_size = atoi(optarg);
    				if (block_size == 0)
    					exit(1);
    				break;
			default:
				printf("usage: lcg [-v] [-s[BLOCK_SIZE]]\n");
				exit(1);
    		}
    	}
    	printf("Block size %dbytes.\n", block_size);

	if (opt_decode) {
		fp_in = request_fp_open(fp_in, OUTPUT, "rb");
		fp_out = request_fp_open(fp_out, FILENAME, "wb");
		
		if (fp_in != NULL && fp_out != NULL)
			decode(fp_in, fp_out, block_size);
		
	} else {
		fp_in = request_fp_open(fp_in, FILENAME, "rb");
		fp_out = request_fp_open(fp_out, OUTPUT, "wb");

		if (fp_in != NULL && fp_out != NULL)
			encode(fp_in, fp_out, block_size);
	}
	
	if (fp_in != NULL)
		fclose(fp_in);
	
	if (fp_out != NULL)
		fclose(fp_out);

}
