// SPDX-License-Identifier: MIT
/*  
 *  main.c - Logistic Chaos bit Generator sample program
 *
 *  This program encodes binary data into logistic map
 *  starting from the initial value x0.
 *
 *  Copyright 2019 Hideto Manjo.
 *  Written by:  Hideto Manjo <enjinit@gmail.com>
 * 
*/
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <time.h>
#include "lcg.h"

/**
 * shred() - Overwrites with random data and removes.
 * @filename  	filename
 *
 * Returns:
 *  1: success
 *  0: fail
 */
static int shred(char *filename)
{
	FILE *fp;
	size_t i = 0;
	unsigned long int r;
	struct stat sb;

	srandom(time(NULL));

	if (stat(filename, &sb) == -1)
		return 0;

	fp = fopen(filename, "wb");
	if (fp == NULL)
		return 0;

	i = sb.st_size / sizeof(unsigned long int);
	while (i--) {
		r = random();
		if (!fwrite(&r, sizeof(unsigned long int), 1, fp))
			return 0;
	}

	i = sb.st_size % sizeof(unsigned long int);
	if (i) {
		r = random();
		if (!fwrite(&r, 1, i, fp))
			return 0;
	}
	
	fclose(fp);

	if (remove(filename) != 0)
		return 0;

	return 1;
}

/**
 * fp_init() - Request file open and initilize.
 *
 * @fp:        pointer of (file pointer)
 * @filename:  filename
 * @mode:      mode
 *
 * Print the error message to stderr and exit(1) when the program
 * can not open file.
 * 
 * Returns:
 *  1: success
 *  0: fail
 */
static void fp_init(FILE **fp, const char *filename, const char *mode)
{
	if ((*fp = fopen(filename, mode)) == NULL) {
		fprintf(stderr, "File open error: %s.\n", filename);
		exit(EXIT_FAILURE);
	}
	/*change buffer size*/
	// setvbuf(fp, NULL, _IOFBF, 512*1024);
}

/**
 * fp_close() - Shorthand function of fclose()
 *
 * This function do not exec fclose() when fp has NULL pointer.
 */
static void fp_close(FILE *fp)
{
	if (fp != NULL)
		fclose(fp);		
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
static int check_file_type(struct stat *sb)
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

	/*for xor in/out*/
	FILE *fp_xor_key;
	FILE *fp_xor_bin;

	struct stat sb_in;
	struct stat sb_out;
	struct lcg_operation_result res = {0};
	struct lcg_operation_result *result = &res; 

	/*option*/
	int opt;
	int opt_decode = 0;
	unsigned int block_size = BLOCK_SIZE;
	int opt_xor = 0;
	char *xor_filename = NULL;
	char xor_tmp_filename[256];

	char *input_filename;
	char *output_filename;

	while ((opt = getopt(argc, argv, "ds:k:")) != -1) {
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
			case 'k':
				opt_xor = 1;
				xor_filename = optarg;
				break;
			default:
				printf("usage: %s [-d] [-k] [-s[BLOCK_SIZE]] \
input output\n", argv[0]);
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


	printf("File size:  %17lldbytes\n", 
			      (long long) sb_in.st_size);

	printf("Block size: %17ubytes\n", block_size);

	if (opt_decode) {
		/*decode*/
		if (opt_xor) {
			strncpy(xor_tmp_filename, input_filename, 248);
			strcat(xor_tmp_filename, ".tmp");

			fp_init(&fp_in, input_filename, "rb");
			fp_init(&fp_xor_key, xor_filename,"rb");
			fp_init(&fp_xor_bin, xor_tmp_filename, "wb");

			lcg_join_xor(fp_in, fp_xor_key, fp_xor_bin);

			fp_close(fp_in);
			fp_close(fp_xor_key);
			fp_close(fp_xor_bin);
		}

		if (opt_xor) {
			fp_init(&fp_in, xor_tmp_filename, "rb");
		} else {
			fp_init(&fp_in, input_filename, "rb");
		}
		
		fp_init(&fp_out, output_filename, "wb");
		
		lcg_decode(fp_in, fp_out, block_size, result);
		printf("Decode: %ldblocks (%ldbytes -> %ldbytes)\n",
						result->block_count,
        					result->read_bytes,
        					result->write_bytes);

		fp_close(fp_in);
		fp_close(fp_out);

		if (opt_xor)
			shred(xor_tmp_filename);

		return 1;

	}

	/*encode*/
	fp_init(&fp_in, input_filename, "rb");
	fp_init(&fp_out, output_filename, "wb");

	lcg_encode(fp_in, fp_out, block_size, result);
	printf("Total %17llubytes\n", result->count);
	printf("Encode %ldblocks (%ldbytes -> %ldbytes)\n",
					result->block_count,
					result->read_bytes,
					result->write_bytes);

	fp_close(fp_in);
	fp_close(fp_out);

	if (opt_xor) {
		/*rename and input*/
		strncpy(xor_tmp_filename, output_filename, 248);
		strcat(xor_tmp_filename, ".tmp");
		if (rename(output_filename, xor_tmp_filename) != 0)
			exit(EXIT_FAILURE);

		fp_init(&fp_in, xor_tmp_filename, "rb");
		fp_init(&fp_xor_key, xor_filename, "wb");
		fp_init(&fp_xor_bin, output_filename, "wb");

		lcg_split_xor(fp_in, fp_xor_key, fp_xor_bin);

		fp_close(fp_in);
		fp_close(fp_xor_key);
		fp_close(fp_xor_bin);

		shred(xor_tmp_filename);
	}

	return 1;
}
