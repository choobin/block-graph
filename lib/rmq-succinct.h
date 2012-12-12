/* Copyright (C) 2008, Johannes Fischer, all rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef RMQ_succinct_hpp
#define RMQ_succinct_hpp

#include <math.h>
#include <stdio.h>

typedef int DT;                 // use long for 64bit-version (but take care of fast log!)
typedef unsigned int DTidx;     // for indexing in arrays

#include <stdlib.h>
#include <limits.h>

typedef unsigned char DTsucc;
typedef unsigned short DTsucc2;

class RMQ_succinct {
public:
	// liefert RMQ[i,j]
	virtual DTidx query(DTidx, DTidx);

	RMQ_succinct(DT* a, DTidx n);

	virtual ~RMQ_succinct();

protected:
	// array
	DT *a;

	// size of array a
	DTidx n;

	// table M for the out-of-block queries (contains indices of block-minima)
	DTsucc** M;

	// because M just stores offsets (rel. to start of block), this method
	// re-calculates the true index:
	inline DTidx m(DTidx k, DTidx b) { return M[k][b]+(b*sprime); }

	// depth of table M:
	DTidx M_depth;

	// table M' for superblock-queries (contains indices of block-minima)
	DTidx** Mprime;

	// depth of table M':
	DTidx Mprime_depth;

	// type of blocks
	DTsucc2 *type;

	// precomputed in-block queries
	DTsucc** Prec;

	// microblock size
	DTidx s;

	// block size
	DTidx sprime;

	// superblock size
	DTidx sprimeprime;

	// number of blocks (always n/sprime)
	DTidx nb;

	// number of superblocks (always n/sprimeprime)
	DTidx nsb;

	// number of microblocks (always n/s)
	DTidx nmb;

	// return microblock-number of entry i:
	inline DTidx microblock(DTidx i) { return i/s; }

	// return block-number of entry i:
	inline DTidx block(DTidx i) { return i/sprime; }

	// return superblock-number of entry i:
	inline DTidx superblock(DTidx i) { return i/sprimeprime; }

	// precomputed Catalan triangle (17 is enough for 64bit computing):
	static const DTidx Catalan[17][17];

	// minus infinity (change for 64bit version)
	static const DT minus_infinity;

 	// stuff for clearing the least significant x bits (change for 64-bit computing)
	static const DTsucc HighestBitsSet[8];
	virtual DTsucc clearbits(DTsucc, DTidx);

	// Least Significant Bits for 8-bit-numbers:
	static const char LSBTable256[256];

    // return least signigicant bit in constant time (change for 64bit version)
	virtual DTidx lsb(DTsucc);

	// the following stuff is for fast base 2 logarithms:
	// (currently only implemented for 32 bit numbers)
	static const char LogTable256[256];

	virtual DTidx log2fast(DTidx);
};

#endif
