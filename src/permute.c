/*
    permutation to "randomly" jump around the on disc

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


    FIXME: describe the algorithm here

*/

#include "permute.h"

#include <stdlib.h>
#include <assert.h>

typedef struct permute_info_offsettbl_ permute_info_offsettbl;

struct permute_info_offsettbl_ {
	permute_info_offsettbl* next;
	unsigned long long N;
	unsigned long long Off[];
};

struct permute_info_ {
	unsigned long long D;
	unsigned int rounds;
	unsigned long long **T;
	permute_info_offsettbl *Off;
};

unsigned long long permute_once(unsigned long long i, 
				unsigned long long N, 
				permute_info *pi);


permute_info* permute_init(unsigned long long D,
			   unsigned int rounds)
{
	unsigned long long i, j, r;
	unsigned long long t;
	permute_info* pi = malloc(sizeof(permute_info));
	pi->D = D;
	pi->rounds = rounds;
	pi->T = malloc(sizeof(unsigned long long *) * (pi->D+1));
	for ( i = 2; i <= pi->D; i++ ) {
		pi->T[i] = malloc(sizeof(unsigned long long)*i);
		for ( j = 0; j < i; j++ )
			pi->T[i][j] = j;
		for ( j = 0; j < i; j++ ) {
			r = j + rand() * (i-j) / RAND_MAX;
			t = pi->T[i][j];
			pi->T[i][j] = pi->T[i][r];
			pi->T[i][r] = t;
		}
	}
	pi->Off = NULL;
	return pi;
}

void permute_free(permute_info *pi)
{
	unsigned int i;
	permute_info_offsettbl* po, *npo;
	for ( i = 2; i <= pi->D; i++ )
		free(pi->T[i]);
	free(pi->T);
	po = pi->Off;
	while ( po ) {
		npo = po->next;
		free(po);
		po = npo;
	}
	free(pi);
}


unsigned long long permute_getoffset(unsigned long long m, 
				     unsigned long long N, 
				     permute_info *pi)
{
	permute_info_offsettbl* po;
	unsigned long long i, d, off;
	assert(m < pi->D);
	po = pi->Off;
	while ( po ) {
		if ( po->N == N )
			break;
		po = po->next;
	}
	if ( ! po ) {
		po = malloc(sizeof(permute_info_offsettbl) +
			    sizeof(unsigned long long) * pi->D);
		po->next = pi->Off;
		pi->Off = po;
		po->N = N;

		off = 0;
		for ( i = 0; i < pi->D; i++ ) {
			d = permute_once(i,pi->D,pi);
			po->Off[d] = off;
			off += N/pi->D;
			if ( d < N % pi->D )
				off++;
		}
	}
	return po->Off[m];
}

unsigned long long permute_once(unsigned long long i, 
				unsigned long long N, 
				permute_info *pi)
{
	assert(i < N);
	if ( N == 1 ) {
		return 0;
	} else if ( N <= pi->D ) {
		return pi->T[N][i];
	} else {
		unsigned long long m = i % pi->D;
		unsigned long long d = i / pi->D;
		unsigned long long w = N / pi->D + ( m < N % pi->D ? 1 : 0 );
		return permute_getoffset(m,N,pi) + permute_once((d+m)%w,w,pi);
	}
}


unsigned long long permute(unsigned long long i, 
			   unsigned long long N, 
			   permute_info *pi)
{
	unsigned int r;
	for ( r = 0; r < pi->rounds; r++ )
		i = permute_once(i,N,pi);
	return i;
}
