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

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <getopt.h>

#define AI 	   	           4  /*initial a, AI=4 means complete chaos*/
#define BLOCK_SIZE 		   2  /*default block size (bytes)*/

#define USE_DBL                    1  /*double or float*/

#define BCD(c)	(5 * (5 * (5 * (5 * (5 * (5 * (5 * (c & 128) + (c & 64)) \
	+ (c & 32)) + (c & 16)) + (c & 8)) + (c & 4)) + (c & 2)) + (c & 1))

#if USE_DBL
#define X_FLOAT_TYPE          double  /*x presision*/
#define BITWISE            bitwise64  /*union bitwise*/
#else
#define X_FLOAT_TYPE           float 
#define BITWISE            bitwise32
#endif

static unsigned long long int seed = 88172645463325252LL;

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
 * union bitwise64 - View of 64bit float point number
 */
union bitwise32 {
	float f;
	long int i;
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
	int i;
	printf("%8s bin: ", title);
	for (i = 0; i < size; i++) {
		printf("%08d ", BCD(byte_array[i]));
	}
	printf("\n%8s hex: ", title);
	for (i = 0; i < size; i++) {
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
 * initilize_seed() - Initilize seed of xor64(). 
 */
void initilize_seed(void)
{
	srandom((unsigned int) time(NULL));
	seed ^= (unsigned long long int) random();
	seed = seed << 23;
	seed ^= (unsigned long long int) random();
}


/**
 * xor64() - Create 64bit pseudorandom numbers using xorshift method
 * 
 * Period of pseudo random number is 2^64 -1.
 *
 */
unsigned long long xor64(void)
{
	unsigned long long *x = &seed;

	*x ^= (*x << 13);
	*x ^= (*x >> 7);
	return (*x ^= (*x << 17));
}

/**
 * uniform_rand() - Generate a uniform random number normalized from 0 to 1.
 */
X_FLOAT_TYPE uniform_rand(void)
{	
	return (
		((X_FLOAT_TYPE) xor64() + 1.0)
		/ ((X_FLOAT_TYPE) ULLONG_MAX + 2.0)
	);
}

/**
 * logistic() - Return logistic map.
 */
X_FLOAT_TYPE logistic(const X_FLOAT_TYPE *x)
{       
	return AI * (*x) * (1 - (*x));
}

/**
 * degitize() - Digitize to 1bit.
 * 
 * This function return unsigned char(0 or 1).
 * 
 */
unsigned char degitize(const X_FLOAT_TYPE *x)
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
unsigned char digitize_char(const X_FLOAT_TYPE *x)
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
void eight_bit(X_FLOAT_TYPE *x0, unsigned char *byte_array)
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
int eight_bit_r(X_FLOAT_TYPE *x0, unsigned char *byte_array,
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
 * one_byte() - Write 8 bits from the initial value of the logistic map.
 *
 * @x0:          initial x
 * @byte_array:  output array
 *
 * This function uses digitize_char().
 */
void one_byte(X_FLOAT_TYPE *x0, unsigned char *byte_array)
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
 *  >=1: success(read size)
 *  0: fail
 */
int read_block(FILE *fp, unsigned char* bin, unsigned int size)
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
int write_block(FILE *fp, unsigned char* bin, unsigned int size)
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
int encode_block(long int *counter, X_FLOAT_TYPE *x0,
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

		if (*counter % 100 == 0)
			printf("\rtry %17ld bytes, match: %4d", 
			       *counter, max_j + 1);

		if (
			j == size - 1
			&& cmp_bytes(&bin[0], &byte_array[0], size)
		) {	
			printf("\rhit %17ld bytes, match: %4d\n", 
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
int encode(FILE *fp_in, FILE *fp_out, const unsigned int block_size)
{	
	const int sizeof_x = sizeof(X_FLOAT_TYPE);

	/*target binary*/
	unsigned char bin[block_size];
	/*generated binary*/                 
	unsigned char byte_array[block_size];
	/*union for X_FLOAT_TYPE to char*/
	union BITWISE bitview;

	X_FLOAT_TYPE x0;
	long int i;
	int block_no = 0;

	unsigned int read_length;
	while((read_length = read_block(fp_in, &bin[0], block_size))) {
		i = 0;
		encode_block(&i, &x0, &bin[0],  &byte_array[0], block_size);
	
		bitview.f = x0;	
		if (read_length < block_size) {
			bitview.c[sizeof_x - 1] = 0x40 + read_length;
		}
		write_block(fp_out, &bitview.c[0], sizeof_x);
		
		block_no++;
	}

	printf("Encode %dblocks(%dbytes).\n",
	       block_no,
	       (block_no - 1) * (sizeof_x) + read_length);
	
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
int decode(FILE *fp_in, FILE *fp_out, const unsigned int block_size)
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

	int block_no = 0;
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
		block_no++;
	}
	/*last block size is write_block_size*/
	printf("Decode %dblocks(%dbytes).\n", block_no,
	        	(block_no - 1) * (block_size) + write_block_size);

	return 1;
}

/**
 * request_fp_open() - Request file open and initilize.
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
	/*change buffer size*/
	// setvbuf(fp, NULL, _IOFBF, 512*1024);
	return fp;
}

/**
 * check_file_type() - Check I/O file type.
 *
 * @sb:		file stat
 *
 * Returns:
 *  1: allow
 *  0: ban
 */
int check_file_type(struct stat *sb)
{
	switch (sb->st_mode & S_IFMT) {
		case S_IFBLK:		/*block device*/
			printf("block device\n");
			return 1;
		case S_IFCHR:		/*character device*/
			printf("character device\n");
			return 1;
		case S_IFDIR:		/*directory*/
			return 0;
		case S_IFIFO:		/*FIFO-pipe*/
			printf("FIFO/pipe\n");
			return 0;
		case S_IFLNK:		/*link*/
			return 0;
		case S_IFREG:		/*regular file*/
			return 1;
		case S_IFSOCK:		/*socket*/
			return 0;
		default:		/*unknown*/
			return 0;
	}

	return 0;
}

int main (int argc, char **argv)
{       
	FILE *fp_in;
	FILE *fp_out;
	struct stat sb_in;
	struct stat sb_out;

	int opt;
	int opt_decode = 0;
	unsigned int block_size = BLOCK_SIZE;

	char *input_filename;
	char *output_filename;

	initilize_seed();

	while ((opt = getopt(argc, argv, "ds:o:")) != -1) {
    		switch (opt) {
    			case 'd':
    				opt_decode = 1;
    				break;
    			case 's':
    				block_size = atoi(optarg);
    				if (
    					block_size == 0
    					|| block_size > UCHAR_MAX - 0x40
    				) {
    					fprintf(stderr,
    						"Invalid block size.\n");
    					exit(EXIT_FAILURE);
    				}
    				break;
			default:
				printf("usage: %s [-v] [-s[BLOCK_SIZE]] \
				                        input output\n",
				                        argv[0]);
				exit(EXIT_FAILURE);
    		}
    	}

	if (argc - optind > 2) {
		fprintf(stderr, "Too many argments\n");
		exit(EXIT_FAILURE);
	}

	if (argc - optind == 0 || strcmp(argv[optind], "-") == 0) {
		fprintf(stderr, "Missing input filename\n");
		exit(EXIT_FAILURE);
	}

	if (argc - optind == 1 || strcmp(argv[optind + 1], "-") == 0) {
		fprintf(stderr,"Missing output filename\n");
		exit(EXIT_FAILURE);
	}

	input_filename = argv[optind];
	output_filename = argv[optind + 1];

	if (stat(input_filename, &sb_in) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	if (!check_file_type(&sb_in)) {
		fprintf(stderr, "input filetype\n");
		exit(EXIT_FAILURE);
	}

	if (stat(output_filename, &sb_out) != -1) {
		if(!check_file_type(&sb_out)) {
			fprintf(stderr, "output filetype\n");
			exit(EXIT_FAILURE);
		}	
	}

	if (sb_in.st_ino == sb_out.st_ino) {
		fprintf(stderr, "input = output\n");
		exit(EXIT_FAILURE);
	}


	printf("File size:                %lld bytes\n", 
			      (long long) sb_in.st_size);

    	printf("Block size %dbytes.\n", block_size);

	if (opt_decode) {
		fp_in = request_fp_open(fp_in, input_filename, "rb");
		fp_out = request_fp_open(fp_out, output_filename, "wb");
		
		if (fp_in != NULL && fp_out != NULL)
			decode(fp_in, fp_out, block_size);
		
	} else {
		fp_in = request_fp_open(fp_in, input_filename, "rb");
		fp_out = request_fp_open(fp_out, output_filename, "wb");

		if (fp_in != NULL && fp_out != NULL)
			encode(fp_in, fp_out, block_size);
	}
	
	if (fp_in != NULL)
		fclose(fp_in);
	
	if (fp_out != NULL)
		fclose(fp_out);

}
