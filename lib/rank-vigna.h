#ifndef RANK_VIGNA_H
#define RANK_VIGNA_H

#include <stdio.h>
#include <stdint.h>

class RankVigna
{
public:
    RankVigna(uint64_t*, size_t);

    ~RankVigna();

    uint64_t rank(uint64_t) const;

private:
    RankVigna();
    RankVigna(const RankVigna&);
    RankVigna& operator=(const RankVigna&);

    static const uint64_t word_bits = 64; // CHARBIT * sizeof uint64_t

    static const uint64_t superblock_bits = 512;

    static const uint64_t superblock_words = 8; // superblock_bits / word_bits

    static const uint64_t block_bits = 9; // log2(superblock_bits)

    inline uint64_t popcount(uint64_t) const;

    size_t nbits;

    uint64_t *bits;

    size_t nblocks;

    uint64_t *block;
};

#endif /* RANK_VIGNA_H */
