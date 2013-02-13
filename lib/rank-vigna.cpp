#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "rank-vigna.h"

// Sebastiano Vigna. Broadword implementation of rank/select
// queries. In Catherine C. McGeoch, editor, Experimental
// Algorithms. 7th International Workshop, WEA 2008, number 5038 in
// Lecture Notes in Computer Science, pages 154−168. Springer-Verlag,
// 2008.

// Donald E. Knuth. The Art of Computer Programming. Pre-Fascicle
// 1A. Draft of Section 7.1.3: Bitwise Tricks and Techniques, 2007.

RankVigna::RankVigna(uint64_t *array, size_t n)
{
    assert(array);
    assert(n > 0);

    bits = array;

    nbits = n;

    nblocks = ((nbits + superblock_bits - 1) / superblock_bits) * 2;

    block = new uint64_t[nblocks + 1]();

    uint64_t nwords = (nbits + word_bits - 1) / word_bits;

    uint64_t count = 0;

    uint64_t index = 0;

    uint64_t i = 0;

    while (i < nwords) {
        block[index] = count;

        count += popcount(bits[i]);

        uint64_t j = 1;

        while (j < superblock_words && i + j < nwords) {
            uint64_t block_count = count - block[index];

            block[index + 1] |= block_count << block_bits * (j - 1);

            count += popcount(bits[i + j]);

            j++;
        }

        i += superblock_words;

        index += 2;
    }

    block[nblocks] = count;

    assert(index == nblocks);
    assert(count <= nbits);
}

RankVigna::~RankVigna()
{
    delete[] block;
}

// If the CPU supports POPCNT or SSE3's PSHUFB instruction we should
// call __builtin_popcountll. Otherwise, use Knuth's sideways addition
// method.

// Note that POPCNT is not actually considered part of SSE4.2,
// however, it was introduced at the same time. In fact, POPCNT and
// LZCNT have their own dedicated CPUID bits to indicate support,
// hence checking for __POPCNT__ and not __SSE4_2__.

uint64_t RankVigna::popcount(uint64_t k) const
{
#if defined __POPCNT__ || defined __SSE3__
    return __builtin_popcountll(k);
#else
    register uint64_t x = k - ((k & 0xAAAAAAAAAAAAAAAA) >> 1);
    x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0F;
    return x * 0x0101010101010101 >> 56;
#endif
}

uint64_t RankVigna::rank(uint64_t k) const
{
    assert(k < nbits);

    uint64_t block_index = (k / superblock_bits) * 2;

    uint64_t bits_index = k / word_bits;

    uint64_t offset = (bits_index % superblock_words) - 1;

    uint64_t mask = (1ULL << (k % word_bits)) - 1;

    // The second level block counts are only required for blocks 1 to
    // 7. We can avoid a test and branch for block 0 with a little bit
    // wizardry.

    // Paraphrased from Vigna2008
    // When bits_index % 8 == 0, the expression ((offset >> 60) & 8)
    // has value 8, which implies that the second level block is
    // shifted by 63, obtaining zero (we are not using the most
    // significant bit of each second level block (as each block
    // contains 7 - 9-bit values packed from LSB to MSB)).

    return
    block[block_index] +
  ((block[block_index + 1] >> (offset + ((offset >> 60) & 8)) * 9) & 0x1ff) +
    popcount(bits[bits_index] & mask);
}
