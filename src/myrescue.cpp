#define __USE_LARGEFILE64 1
#define _LARGEFILE_SOURCE 1
#define _LARGEFILE64_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int copy_block(int src_fd, int dst_fd, int blocknum, int blocksize)
{
	unsigned char buffer[blocksize];
	long long filepos = blocknum;

	filepos *= blocksize;

	if (lseek64(src_fd, filepos, SEEK_SET) < 0) {
		perror("lseek64 src_fd");
		return errno;
	}

	if (lseek64(dst_fd, filepos, SEEK_SET) < 0) {
		perror("lseek64 dst_fd");
		return errno;
	}

	ssize_t count = read(src_fd, buffer, blocksize);
	if (count != blocksize) {
		perror("src read failed");
		return errno;
	}
		
	ssize_t tgt_count = write(dst_fd, buffer, blocksize);
	if (tgt_count != blocksize) {
		perror("dst write failed");
		return errno;
	}
	return 0;
}

long long filesize(int device)
{
	long long rval(lseek64(device, 0, SEEK_END));
	if (rval < 0) {
		perror("filesize");
		exit(-1);
	}
	return rval;
}

int peek_map(int bitmap_fd, int block)
{
	char c = 0;
	if (lseek64(bitmap_fd, block, SEEK_SET) < 0) {
		perror("peek_map lseek64");
		exit(-1);
	}
	int count = read(bitmap_fd, &c, 1);
	if (count < 0) {
		perror("peek_map read");
		exit(-1);
	}
	return c;
}

void poke_map(int bitmap_fd, int block, char val)
{
	if (lseek64(bitmap_fd, block, SEEK_SET) < 0) {
		perror("poke_map lseek64");
		exit(-1);
	}
	if (write(bitmap_fd, &val, 1) != 1) {
		perror("poke_map write");
		exit(-1);
	}
}

void doit(int dev_fd, int backup_fd, int bitmap_fd, int blocksize,
	  bool sweep_mode, int block_retry, int start_block)
{
	ssize_t blocks = filesize(dev_fd) / blocksize;
	ssize_t block_skip = 1;

	long error_count = 0;
        long read_count = 0;
	long i;
	for (i = start_block; i < blocks; i += block_skip) { 
		char c = peek_map(bitmap_fd, i);
		if (c <= 0) {
			int rv ;
			for ( int r = 0 ; r < block_retry ; r++ ) {
				fprintf(stderr, "\rBlock: %09ld/%09ld "
					"Read: %09ld Error: %09ld   ",
					i, blocks, read_count, error_count);
				rv = copy_block(dev_fd,backup_fd,i,blocksize);
				if ( rv == 0 ) {
					break ;
				}
			} 
			read_count++;
			if (rv == 0) {
				poke_map(bitmap_fd, i, 1);
				if (sweep_mode) {
					block_skip = 1;
				}
			} else {
				error_count++;
				poke_map(bitmap_fd, i, c-1);
				if (sweep_mode) {
					block_skip *= 2;
				}
			}
		} else {
			if ( i % 1000 == 0 ) {
				fprintf(stderr, "\rBlock: %09ld/%09ld "
					"Read: %09ld Error: %09ld   ",
					i, blocks, read_count, error_count);
			}
			block_skip = 1;
		}
	}
	fprintf(stderr, "\rBlock: %09ld/%09ld "
		"Read: %09ld Error: %09ld   \n",
		i, blocks, read_count, error_count);
}

int main(int argc, const char** argv)
{
	if (argc < 7) {
		fprintf(stderr, "Usage: input-file output-file bitmap-file "
			"block-size sweep-mode block-retry [start-block]\n");
		exit(-1);
	}
	int dev_fd = open64(argv[1], O_RDONLY);
	int backup_fd = open64(argv[2], O_RDWR | O_CREAT, 0600);
	int bitmap_fd = open64(argv[3], O_RDWR | O_CREAT, 0600);

	if (dev_fd < 0 || backup_fd < 0 || bitmap_fd < 0) {
		fprintf(stderr, "Error opening files!\n");
		exit(-1);
	}
	int blocksize = atol(argv[4]);
	if (blocksize <= 0) {
		fprintf(stderr, "Blocksize invalid: %d!\n", blocksize);
		exit(-1);
	}

	int sweepmode = atol(argv[5]);
	if (sweepmode < 1 || sweepmode > 2) {
		fprintf(stderr, "Invalid sweepmode: %d!\n", sweepmode);
		exit(-1);
	}

	int block_retry = atol(argv[6]);
	if (block_retry <= 0) {
		fprintf(stderr, "Block_retry invalid: %d!\n", block_retry);
		exit(-1);
	}

	int start_block = 0;
	if (argc > 7) {
		start_block = atol(argv[7]);
	}
	if (start_block < 0) {
		fprintf(stderr, "start block invalid: %d!\n", start_block);
		exit(-1);
	}

	doit(dev_fd, backup_fd, bitmap_fd, blocksize, (sweepmode == 1),
	     block_retry, start_block);
	
}
