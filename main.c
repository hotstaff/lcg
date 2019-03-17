// SPDX-License-Identifier: MIT
/*  
 *  lcg.c - Logistic Chaos bit Generator sample program
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
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include "lcg.h"


/**
 * fp_init() - Request file open and initilize.
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
static FILE *fp_init(const char *filename, const char *mode)
{
	FILE *fp;
	if ((fp = fopen(filename, mode)) == NULL) {
		fprintf(stderr, "File open error: %s.\n", filename);
		return fp;
	}
	/*change buffer size*/
	// setvbuf(fp, NULL, _IOFBF, 512*1024);
	return fp;
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

		if(opt_xor) {
			/*rename and in*/
			strncpy(xor_tmp_filename, input_filename, 248);
			strcat(xor_tmp_filename, ".tmp");

			fp_in = fp_init(input_filename, "rb");
			fp_xor_key = fp_init(xor_filename,"rb");
			fp_xor_bin = fp_init(xor_tmp_filename, "wb");

			if (fp_in != NULL 
			    && fp_xor_key != NULL
			    && fp_xor_bin != NULL) 
				lcg_join_xor(fp_in, fp_xor_key, fp_xor_bin);

			fp_close(fp_in);
			fp_close(fp_xor_key);
			fp_close(fp_xor_bin);
		}

		if (opt_xor) {
			fp_in = fp_init(xor_tmp_filename, "rb");
		} else {
			fp_in = fp_init(input_filename, "rb");
		}
		
		fp_out = fp_init(output_filename, "wb");
		
		if (fp_in != NULL && fp_out != NULL) {
			lcg_decode(fp_in, fp_out, block_size, result);
			printf("Decode: %ldblocks (%ldbytes -> %ldbytes)\n",
				result->block_count,
	        		result->read_bytes,
	        		result->write_bytes);
		}

		fp_close(fp_in);
		fp_close(fp_out);

		if (opt_xor)
			remove(xor_tmp_filename);

	} else {
		fp_in = fp_init(input_filename, "rb");
		fp_out = fp_init(output_filename, "wb");

		if (fp_in != NULL && fp_out != NULL) {
			lcg_encode(fp_in, fp_out, block_size, result);
			printf("Total %17llubytes\n", result->count);
			printf("Encode %ldblocks (%ldbytes -> %ldbytes)\n",
				result->block_count,
	        		result->read_bytes,
	        		result->write_bytes);
		}

		fp_close(fp_in);
		fp_close(fp_out);

		if(opt_xor) {
			/*rename and in*/
			strncpy(xor_tmp_filename, output_filename, 248);
			strcat(xor_tmp_filename, ".tmp");
			if (rename(output_filename, xor_tmp_filename) != 0)
				exit(EXIT_FAILURE);

			fp_in = fp_init(xor_tmp_filename, "rb");
			fp_xor_key = fp_init(xor_filename, "wb");
			fp_xor_bin = fp_init(output_filename, "wb");
			if (fp_in != NULL 
			    && fp_xor_key != NULL
			    && fp_xor_bin != NULL) 
				lcg_split_xor(fp_in, fp_xor_key, fp_xor_bin);

			fp_close(fp_in);
			fp_close(fp_xor_key);
			fp_close(fp_xor_bin);

			remove(xor_tmp_filename);
		}

		
	}

	
}
