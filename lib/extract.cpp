#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "bit-vector.h"
#include "graph.h"
#include "rank.h"
#include "utility.h"

uint8_t Graph::extract_r(size_t i,
                         size_t block,
                         size_t block_length,
                         size_t depth) const
{
    assert(i < block_length);

    size_t child_length = block_length / 2;

    size_t child_distance = child_length / 2;

    bool is_internal_block = level_bits[depth]->access(block);

    size_t rank1 = level_bits[depth]->rank1(block);

    if (block_length <= truncated_length && is_internal_block) {
        size_t block_distance = block_length / 2;

        size_t runs = level_runs[depth]->rank1(block);

        size_t offset = (rank1 * truncated_length) - (runs * block_distance);

        return text_blocks[offset + i];
    }

    size_t index;

    if (i < child_length)
        index = 0; // lhs
    else if (i < child_distance + child_length)
        index = 1; // mid
    else
        index = 2; // rhs

    size_t nexti = i - (index * child_distance);

    if (is_internal_block) {
        size_t runs = level_runs[depth]->rank1(block);

        size_t nextbit = (3 * rank1) - runs + index;

        return extract_r(nexti, nextbit, child_length, depth + 1);
    }
    else { // is_leaf_block
        size_t leaf = block - rank1; // level_bits[depth]->rank0(block);

        size_t pointer = leaf_pointers[depth]->get((3 * leaf) + index);

        size_t offset = leaf_offsets[depth]->get((3 * leaf) + index);

        return extract_r(nexti + offset, block - pointer, block_length, depth);
    }

    return 0;
}

uint8_t Graph::extract(size_t i) const
{
    assert(i < nbytes_text);

    return extract_r(i, 0, next_pow2(nbytes_text),  0);
}

void Graph::extract_r(uint8_t *str,
                      size_t i,
                      size_t j,
                      size_t block,
                      size_t block_length,
                      size_t depth) const
{
    assert(str);
    assert(i <= j);
    assert(j < block_length);

    size_t child_length = block_length / 2;

    size_t child_distance = child_length / 2;

    bool is_internal_block = level_bits[depth]->access(block);

    size_t rank1 = level_bits[depth]->rank1(block);

    if (block_length <= truncated_length && is_internal_block) {
        size_t block_distance = block_length / 2;

        size_t runs = level_runs[depth]->rank1(block);

        size_t offset = (rank1 * truncated_length) - (runs * block_distance);

        memcpy(str, text_blocks + offset + i, j - i + 1);

        return;
    }

    size_t index;

    bool split = false;

    if (i >= child_distance &&
        j < child_distance + child_length) {
        index = 1; // Text is completely contained in middle block.
    }
    else if (i < child_length) {
        index = 0; // Text begins in the first block.

        if (j >= child_length) { // Text is also in rhs block (have to split).
            split = true;

            if (j < child_distance + child_length)
                index = 1; // The rest of the text is contained in the mid block
            else
                index = 2; // The rest of the text is in the rhs block.
        }
    }
    else // if (i >= (child_distance * 2))
        index = 2; // Text is completely contained in the rhs block.

    uint8_t *nextstr;
    size_t nexti;
    size_t nextj;
    size_t nextblock;
    size_t nextlength;
    size_t nextdepth;

    size_t runs;
    size_t leaf;

    if (is_internal_block)
        runs = level_runs[depth]->rank1(block);
    else // is_leaf_block
        leaf = block - rank1; // level_bits[depth]->rank0(block);

    if (split) {
        // Calculate split.
        nexti = i;
        nextj = child_length - 1;

        // We do not need to normalize the left block.

        if (is_internal_block) {
            // We want the first child of the leaf block.
            nextblock  = (3 * rank1) - runs;

            nextlength = child_length;

            nextdepth = depth + 1;
        }
        else { // is_leaf_block
            // We want the first child of the leaf block.
            size_t pointer = leaf_pointers[depth]->get(3 * leaf);

            size_t offset = leaf_offsets[depth]->get(3 * leaf);

            nexti += offset;
            nextj += offset;

            nextblock = block - pointer;

            nextlength = block_length;

            nextdepth = depth;
        }

        extract_r(str, nexti, nextj, nextblock, nextlength, nextdepth);
    }

    if (split) {
        // Calculate string offset.
        nextstr = str + (nextj - nexti + 1);

        // Calculate spit.
        nexti = child_length;
        nextj = j;
    }
    else {
        nextstr = str;
        nexti = i;
        nextj = j;
    }

    // Normalize.
    nexti -= (index * child_distance);
    nextj -= (index * child_distance);

    if (is_internal_block) {
        nextblock  = (3 * rank1) - runs + index;

        nextlength = child_length;

        nextdepth = depth + 1;
    }
    else { // is_leaf_block
        size_t pointer = leaf_pointers[depth]->get((3 * leaf) + index);

        size_t offset = leaf_offsets[depth]->get((3 * leaf) + index);

        nexti += offset;
        nextj += offset;

        nextblock = block - pointer;

        nextlength = block_length;

        nextdepth = depth;
    }

    extract_r(nextstr, nexti, nextj, nextblock, nextlength, nextdepth);
}

void Graph::extract(uint8_t *str, size_t i, size_t j) const
{
    assert(str);
    assert(i <= j);
    assert(j < nbytes_text);

    extract_r(str, i, j, 0, next_pow2(nbytes_text), 0);

    str[j - i + 1] = 0;
}

uint8_t *Graph::extract(size_t i, size_t j) const
{
    assert(i <= j);
    assert(j < nbytes_text);

    uint8_t *str = new uint8_t[(j - i + 1) + 1];

    extract_r(str, i, j, 0, next_pow2(nbytes_text), 0);

    str[j - i + 1] = 0;

    return str;
}
