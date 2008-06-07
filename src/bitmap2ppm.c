/*
    Convert myrescue block bitmap to an image
    Copyright (C) 2007  Kristof Koehler (kristofk at users.sourceforge.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define __USE_LARGEFILE64 1
#define _LARGEFILE_SOURCE 1
#define _LARGEFILE64_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv)
{
	char *bitmap_name;
	int fd;
	int arg;

	unsigned long long width = 0;
	unsigned long long height = 0;
	unsigned long long size;
	unsigned long long size_remain;
	char *buf_bit;
	char *buf_rgb;
	
	unsigned long long w, h, x, y;
	int i, n;

	if ( argc < 3 ) {
		fprintf(stderr,"usage: bitmap2ppm <bitmap-file> <width> <rows> <width> ...\n");
		exit(-1);
	}

	bitmap_name = argv[1];

	fd = open64(bitmap_name, O_RDONLY);
	if ( fd < 0 ) {
		perror ( "bitmap open failed" ) ;
		exit(-1) ;
	}

	size = lseek64(fd, 0, SEEK_END) ;
	if (size < 0) {
		perror("filesize");
		exit(-1);
	}

	if ( lseek64(fd, 0, SEEK_SET) < 0 ) {
		perror("seek");
		exit(-1);
	}

	size_remain = size;
	for ( arg = 2; (size_remain > 0) && (arg < argc); arg +=2 ) {
		w = atoi(argv[arg]);
		if ( width < w )
			width = w;
		h = (size_remain + (w-1)) / w;
		if ( arg+1 < argc ) {
			y = atoi(argv[arg+1]);
			if ( y < h )
				h = y;
		}
		height += h;
		size_remain -= w*h;
	}

	fprintf(stdout,"P6\n%llu %llu\n255\n", width, height);
	fflush(stdout);
	
	buf_bit = malloc(width);
	if ( ! buf_bit ) {
		fprintf(stderr,"malloc failed\n");
		exit(-1);
	}

	buf_rgb = malloc(3*width);
	if ( ! buf_rgb ) {
		fprintf(stderr,"malloc failed\n");
		exit(-1);
	}
	
	size_remain = size;
	for ( arg = 2; (size_remain > 0) && (arg < argc); arg +=2 ) {
		w = atoi(argv[arg]);
		h = (size_remain + (w-1)) / w;
		if ( arg+1 < argc ) {
			y = atoi(argv[arg+1]);
			if ( y < h )
				h = y;
		}
		for ( y = 0; y < h; y++ ) {
			n = 0;
			while ( (n < w) && (i = read(fd, buf_bit, w)) > 0 )
				n += i;
			if ( i < 0 ) {
				perror("read failed");
				exit(-1);
			}
			
			for ( x = 0; x < width; x++ ) {
				if ( x >= n ) {
					buf_rgb[3*x+0] = 0;
					buf_rgb[3*x+1] = 0;
					buf_rgb[3*x+2] = 128;
				} else if ( buf_bit[x] == 1 ) {
					buf_rgb[3*x+0] = 0;
					buf_rgb[3*x+1] = 128;
					buf_rgb[3*x+2] = 0;
				} else if ( buf_bit[x] == 2 ) {
					buf_rgb[3*x+0] = 128;
					buf_rgb[3*x+1] = 255;
					buf_rgb[3*x+2] = 0;
				} else if ( buf_bit[x] == 0 ) {
					buf_rgb[3*x+0] = 0;
					buf_rgb[3*x+1] = 0;
					buf_rgb[3*x+2] = 0;
				} else if ( buf_bit[x] == -1 ) {
					buf_rgb[3*x+0] = 255;
					buf_rgb[3*x+1] = 255;
					buf_rgb[3*x+2] = 0;
				} else {
					buf_rgb[3*x+0] = 255;
					buf_rgb[3*x+1] = 0;
					buf_rgb[3*x+2] = 0;
				}
			}
			write(1, buf_rgb, 3*width);
		}
		size_remain -= w*h;
	}
	
	close(fd);
	free(buf_bit);
	free(buf_rgb);

	return 0 ;
}
