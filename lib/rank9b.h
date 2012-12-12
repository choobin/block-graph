/*
 * Sux: Succinct data structures
 *
 * Copyright (C) 2007-2011 Sebastiano Vigna
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the Free
 *  Software Foundation; either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef rank9b_h
#define rank9b_h
#include <stdint.h>
#include <assert.h>

const unsigned char popcount[] = {
0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,
};

#define ONES_STEP_4 ( 0x1111111111111111ULL )
#define ONES_STEP_8 ( 0x0101010101010101ULL )
#define ONES_STEP_9 ( 1ULL << 0 | 1ULL << 9 | 1ULL << 18 | 1ULL << 27 | 1ULL << 36 | 1ULL << 45 | 1ULL << 54 )
#define ONES_STEP_16 ( 1ULL << 0 | 1ULL << 16 | 1ULL << 32 | 1ULL << 48 )
#define MSBS_STEP_4 ( 0x8ULL * ONES_STEP_4 )
#define MSBS_STEP_8 ( 0x80ULL * ONES_STEP_8 )
#define MSBS_STEP_9 ( 0x100ULL * ONES_STEP_9 )
#define MSBS_STEP_16 ( 0x8000ULL * ONES_STEP_16 )
#define INCR_STEP_8 ( 0x80ULL << 56 | 0x40ULL << 48 | 0x20ULL << 40 | 0x10ULL << 32 | 0x8ULL << 24 | 0x4ULL << 16 | 0x2ULL << 8 | 0x1 )

#define ONES_STEP_32 ( 0x0000000100000001ULL )
#define MSBS_STEP_32 ( 0x8000000080000000ULL )

#define COMPARE_STEP_8(x,y) ( ( ( ( ( (x) | MSBS_STEP_8 ) - ( (y) & ~MSBS_STEP_8 ) ) ^ (x) ^ ~(y) ) & MSBS_STEP_8 ) >> 7 )
#define LEQ_STEP_8(x,y) ( ( ( ( ( (y) | MSBS_STEP_8 ) - ( (x) & ~MSBS_STEP_8 ) ) ^ (x) ^ (y) ) & MSBS_STEP_8 ) >> 7 )

#define UCOMPARE_STEP_9(x,y) ( ( ( ( ( ( (x) | MSBS_STEP_9 ) - ( (y) & ~MSBS_STEP_9 ) ) | ( x ^ y ) ) ^ ( x | ~y ) ) & MSBS_STEP_9 ) >> 8 )
#define UCOMPARE_STEP_16(x,y) ( ( ( ( ( ( (x) | MSBS_STEP_16 ) - ( (y) & ~MSBS_STEP_16 ) ) | ( x ^ y ) ) ^ ( x | ~y ) ) & MSBS_STEP_16 ) >> 15 )
#define ULEQ_STEP_9(x,y) ( ( ( ( ( ( (y) | MSBS_STEP_9 ) - ( (x) & ~MSBS_STEP_9 ) ) | ( x ^ y ) ) ^ ( x & ~y ) ) & MSBS_STEP_9 ) >> 8 )
#define ULEQ_STEP_16(x,y) ( ( ( ( ( ( (y) | MSBS_STEP_16 ) - ( (x) & ~MSBS_STEP_16 ) ) | ( x ^ y ) ) ^ ( x & ~y ) ) & MSBS_STEP_16 ) >> 15 )
#define ZCOMPARE_STEP_8(x) ( ( ( x | ( ( x | MSBS_STEP_8 ) - ONES_STEP_8 ) ) & MSBS_STEP_8 ) >> 7 )


class rank9b {
private:
	const uint64_t *bits;
	uint64_t *counts, *inventory;
	uint64_t num_words, num_counts, inventory_size, ones_per_inventory, log2_ones_per_inventory, num_ones;

	/** Counts the number of bits in x. */
	__inline static int count( const uint64_t x ) {
		register uint64_t byte_sums = x - ( ( x & 0xa * ONES_STEP_4 ) >> 1 );
		byte_sums = ( byte_sums & 3 * ONES_STEP_4 ) + ( ( byte_sums >> 2 ) & 3 * ONES_STEP_4 );
		byte_sums = ( byte_sums + ( byte_sums >> 4 ) ) & 0x0f * ONES_STEP_8;
		return byte_sums * ONES_STEP_8 >> 56;
	}

	/* Selects the k-th (k>=0) bit in l. Returns 72 if no such bit exists. */
	__inline static int select_in_word( const uint64_t x, const int k ) {
		assert( k < count( x ) );

		// Phase 1: sums by byte
		register uint64_t byte_sums = x - ( ( x & 0xa * ONES_STEP_4 ) >> 1 );
		byte_sums = ( byte_sums & 3 * ONES_STEP_4 ) + ( ( byte_sums >> 2 ) & 3 * ONES_STEP_4 );
		byte_sums = ( byte_sums + ( byte_sums >> 4 ) ) & 0x0f * ONES_STEP_8;
		byte_sums *= ONES_STEP_8;

		// Phase 2: compare each byte sum with k
		const uint64_t k_step_8 = k * ONES_STEP_8;
		const uint64_t place = ( LEQ_STEP_8( byte_sums, k_step_8 ) * ONES_STEP_8 >> 53 ) & ~0x7;

		// Phase 3: Locate the relevant byte and make 8 copies with incrental masks
		const int byte_rank = k - ( ( ( byte_sums << 8 ) >> place ) & 0xFF );

		const uint64_t spread_bits = ( x >> place & 0xFF ) * ONES_STEP_8 & INCR_STEP_8;
		const uint64_t bit_sums = ZCOMPARE_STEP_8( spread_bits ) * ONES_STEP_8;

		// Compute the inside-byte location and return the sum
		const uint64_t byte_rank_step_8 = byte_rank * ONES_STEP_8;

		return place + ( LEQ_STEP_8( bit_sums, byte_rank_step_8 ) * ONES_STEP_8 >> 56 );
	}


public:
	rank9b( const uint64_t * const bits, const uint64_t num_bits );
	~rank9b();
	uint64_t rank( const uint64_t pos ) const;
	uint64_t select( const uint64_t rank ) const;
	// Just for analysis purposes
	void print_counts();
	uint64_t bit_count();
};

#endif
