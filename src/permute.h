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



    WARNING:

    This algorithm is basically a weekend hack and only intended for
    the purpose of randomizing disc access. It definitely produces a
    permutation, but I have not checked that the output is sufficiently
    random or that the selection of the permutation is unbiased. In
    fact, I'm even quite sure, that it only generates a subset of all
    possible permutations.

    So keep it away from applications that depend on the quality of the
    randomness or the unbiased permutation selection. This includes in
    particular areas like statistics, simulation and cryptography.

 */

#ifndef __PERMUTE_H__INCLUDED__
#define __PERMUTE_H__INCLUDED__

typedef struct permute_info_ permute_info;

permute_info* permute_init(unsigned long long D,
			   unsigned int rounds);

void permute_free(permute_info *pi);

unsigned long long permute(unsigned long long i, 
			   unsigned long long N, 
			   permute_info *pi);

#endif /* __PERMUTE_H__INCLUDED__ */
