/*
    Statistics dumper for myrescue
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

#define BUFFER_SIZE 4096

int main(int argc, char** argv)
{
	char *bitmap_name;
	int bitmap_fd;

	unsigned long long count[256];
	unsigned long long total = 0;
	unsigned char buffer[BUFFER_SIZE];

	int i, n;
	unsigned long long ull;

	if ( argc != 2 ) {
		fprintf(stderr,"usage: myrescue-stat <bitmap-file>\n");
		exit(-1);
	}

	bitmap_name = argv[1];

	bitmap_fd = open64(bitmap_name, O_RDONLY);
	if ( bitmap_fd < 0 ) {
		perror ( "bitmap open failed" ) ;
		exit(-1) ;
	}

	for ( i = 0; i < 256; i++ )
		count[i] = 0;

	while ( (n = read(bitmap_fd, &buffer, BUFFER_SIZE)) > 0 ) {
		for ( i = 0; i < n; i++ )
			count[buffer[i]]++;
		total += n;
	}

	if ( n < 0 ) {
		perror ( "bitmap read failed" );
		exit(-1);
	}

	close(bitmap_fd);

	for ( i = -127; i < 128; i++ ) {
		ull = count[(unsigned char)i];
		if ( ull > 0 )
			fprintf ( stdout, "%d: %llu (%.2f%%)\n",
				  i, ull, 100.0 * ull / total );
	}

	return 0 ;
}
