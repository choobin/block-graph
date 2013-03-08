#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <limits>
#include <string>
#include <vector>
using std::numeric_limits;
using std::basic_string;
using std::vector;

#include "array.h"
#include "bit-vector.h"
#include "graph.h"
#include "rank.h"
#include "stopwatch.h"
#include "text.h"
#include "utility.h"

enum Type {
    internal_block,
    leaf_block,
    text_block
};

struct Block {
    size_t lhs;
    size_t rhs;
    Type type;
};

static const size_t nchildren = 3;

struct Leaf {
    uint32_t pointer[nchildren];
    uint32_t offset[nchildren];
};

struct Statistics {
    struct {
        size_t internal;
        size_t leaf;
        size_t text;
    } nblocks;
    struct {
        size_t bit_vectors;
        size_t leaf_blocks;
        size_t text_blocks;
        size_t total;
    } nbytes;
};

static size_t find(const vector<Block>& level, size_t index, size_t length)
{
    size_t lhs = 0;
    size_t rhs = level.size() - 1;
    size_t mid;

    while (lhs <= rhs) {
        mid = lhs + ((rhs - lhs) >> 1);

        if (index >= level[mid].lhs &&
            index + length - 1 <= level[mid].rhs)
            break;

        if (level[mid].lhs < index)
            lhs = mid + 1;
        else if (level[mid].rhs > index)
            rhs = mid - 1;
    }

    // We could be a middle block. Check to the
    // lhs (if we can) to see if it is suitable.

    if (mid > 0 &&
        index >= level[mid - 1].lhs &&
        index + length - 1 <= level[mid - 1].rhs)
        return mid - 1;

    return mid;
}

static bool is_leaf(const vector<Block>& level, Block& block, Leaf& leaf, Text& text)
{
    size_t block_length = next_pow2(block.rhs - block.lhs + 1);

    size_t child_length = block_length / 2;

    size_t child_distance = child_length / 2;

    size_t nleaf_children = 0;

    for (size_t i = 0; i < nchildren; i++) {
        Block child;

        child.lhs = block.lhs + (i * child_distance);
        child.rhs = child.lhs + child_length - 1;

        // Nothing left to do here.
        if (child.lhs >= block.rhs)
            break;

        // Fix up final blocks rhs value.
        if (child.rhs > block.rhs) {
            child.rhs = block.rhs;
            child_length = child.rhs - child.lhs + 1;
        }

        size_t left_most = text.first_occurrence(child.lhs, child.rhs);

        if (left_most == child.lhs)
            break; // This is not a leaf block.

        assert(left_most < child.lhs);

        size_t k = find(level, left_most, child_length);

        assert(level[k].lhs <= left_most &&
               level[k].rhs >= left_most + child_length - 1);

        size_t offset = left_most - level[k].lhs;

        assert(k <= numeric_limits<uint32_t>::max());

        leaf.pointer[i] = (uint32_t)k;

        assert(offset <= numeric_limits<uint32_t>::max());

        leaf.offset[i] = (uint32_t)offset;

        nleaf_children++;
    }

    size_t nbytes = text.size();

    // In order to be a leaf block
    if (block.lhs > 0 && // we can not be the first block on a specific level
        (nleaf_children == nchildren || // and we either have three children
         (nleaf_children == 2 && // or two, however, we need to be the last
          block.rhs == nbytes - 1 && // block on a level and the
          child_length < next_pow2(child_length) && // text length is not a
          nbytes < next_pow2(nbytes)))) { // power of 2
                                            // wow1! :D

        // If we are at a leaf block with 2 children. Mark it with
        // UINT32_MAX max so know to ignore the final child when we
        // create each level arrays.

        if (nleaf_children == 2) {
            leaf.pointer[2] = numeric_limits<uint32_t>::max();
            leaf.offset[2] = numeric_limits<uint32_t>::max();
        }

        return true;
    }

    return false;
}

void Graph::build()
{
    Text text(path);

    Stopwatch stopwatch("[block-graph build]");

    vector<Block> curr_level;
    vector<Block> next_level;

    Block block;
    block.lhs = 0;
    block.rhs = nbytes_text - 1;
    block.type = internal_block;

    curr_level.push_back(block);

    vector<uint32_t> pointers;
    vector<uint32_t> offsets;

    // For lazy concatenation during construction.
    basic_string<uint8_t> text_blocks_tmp;

    size_t nbytes_padded = next_pow2(nbytes_text);

    size_t level = nbytes_padded / 2;

    size_t index = 0;

    Statistics statistics;
    memset(&statistics, 0, sizeof statistics);

    // The root level is a single internal block.
    BitVector *bv = new BitVector(1);
    bv->set(0);

    level_bits[index] = new Rank(bv);
    level_runs[index] = new Rank(new BitVector(1));

    leaf_pointers[index] = new Array(pointers);
    leaf_offsets[index] = new Array(offsets);

    // To save space we could neglect all top levels of the graph that
    // consist entirely of internal nodes. I doubt that there would be
    // much of a space saving as the leaf blocks dominate the size of
    // the graph. If we /really/ want to squeeze the space we could
    // save a single parameter level_where_leaf_blocks_start or
    // something similar. Note that lib/extract.cpp code would need
    // some hacking to deal with the empty top levels.

    // Follow-up: There was a negligible space saving.

    index++;

    while (level > 1) {
        size_t block_length = level * 2;

        size_t child_length = block_length / 2;

        size_t child_distance = child_length / 2;

        size_t nblocks_internal = 0;

        size_t nblocks_leaf = 0;

        size_t nblocks_text = 0;

        printf("Computing depth %zu with block length %zu",
               index, block_length);

        fflush(stdout); // Gordon! The fastest man in the uuuuuniverse.

        for (size_t i = 0; i < curr_level.size(); i++) {
            block = curr_level[i];

            if (block.type != internal_block)
                continue;

            for (size_t j = 0 ; j < nchildren; j++) {
                Block child;

                child.lhs = block.lhs + (j * child_distance);
                child.rhs = child.lhs + child_length - 1;

                // Nothing left to do here.
                if (child.lhs >= nbytes_text - 1)
                    break;

                // Fix up final blocks rhs value.
                if (child.rhs > nbytes_text - 1) {
                    child.rhs = nbytes_text - 1;

                    // Nothing left to do here.
                    if (child.rhs - child.lhs + 1 <= child_length / 2)
                        break;
                }

                if (i > 0 &&
                    j == 0 &&
                    curr_level[i - 1].type == internal_block &&
                    curr_level[i - 1].lhs + child_length ==
                    curr_level[i].lhs) {

                    // 1) If we are not the first block in the current level.

                    // 2) And we are the first child of the current block.

                    // 3) Make sure that the previous block is not a leaf
                    //    as we can not link back to leaf blocks.

                    // 4) If we get here this child has already been
                    //    created so we can skip to the next child.

                    continue;
                }

                Leaf leaf;

                if (child_length <= truncated_length) {
                    uint8_t tmp[truncated_length + 1];

                    size_t offset = child.lhs;

                    if (next_level.size() > 0) {
                        Block prev = next_level[next_level.size() - 1];

                        // If there are two consecutive text blocks we
                        // can remove the overlap of block_length / 2.

                        if (prev.type == text_block &&
                            prev.lhs + child_distance == child.lhs)
                            offset += child_distance;
                    }

                    text.extract(tmp, offset, child.rhs);
                    tmp[child.rhs - offset + 1] = '\0';
                    text_blocks_tmp += tmp; // <3

                    child.type = text_block;
                    nblocks_text++;
                }
                else if (is_leaf(next_level, child, leaf, text)) {
                    for (size_t k = 0; k < nchildren; k++) {
                        if (leaf.pointer[k] == numeric_limits<uint32_t>::max())
                            break;

                        size_t position = next_level.size();
                        assert(position > leaf.pointer[k]);

                        pointers.push_back(position - leaf.pointer[k]);
                        offsets.push_back(leaf.offset[k]);
                    }

                    child.type = leaf_block;
                    nblocks_leaf++;
                }
                else {
                    child.type = internal_block;
                    nblocks_internal++;
                }

                next_level.push_back(child);

            } // end for ... j < nchildren ...

        } // end for ... i < curr_level.size() ...

        if (child_length > truncated_length)
            printf(" (internal: %zu, leaf: %zu)\n",
                   nblocks_internal, nblocks_leaf);
        else
            printf(" (text: %zu)\n", nblocks_text);

        statistics.nblocks.internal += nblocks_internal;
        statistics.nblocks.leaf += nblocks_leaf;
        statistics.nblocks.text += nblocks_text;

        BitVector *bits = new BitVector(next_level.size());
        BitVector *runs = new BitVector(next_level.size());

        for (size_t i = 0; i < next_level.size(); i++) {
            if (next_level[i].type != leaf_block)
                bits->set(i);

            // This constructs R_d for both internal levels and text block
            // levels. If there are two consecutive internal/text blocks
            // we mark a 1 bit so we can calculate correct offsets
            // during extract.

            if (i < next_level.size() - 1 &&
                next_level[i].type != leaf_block &&
                next_level[i + 1].type != leaf_block &&
                next_level[i].lhs + child_distance ==
                next_level[i + 1].lhs)
                runs->set(i);
        }

        level_bits[index] = new Rank(bits);
        level_runs[index] = new Rank(runs);

        leaf_pointers[index] = new Array(pointers);
        leaf_offsets[index] = new Array(offsets);

#if not defined NDEBUG
        printf("bits size: %zu bytes\n", level_bits[index]->size());
        printf("runs size: %zu bytes\n", level_runs[index]->size());
        printf("pointers size: %zu bytes\n", leaf_pointers[index]->size());
        printf("offsets size : %zu bytes\n\n", leaf_offsets[index]->size());
#endif
        pointers.clear();
        offsets.clear();

        if (child_length <= truncated_length) {
            statistics.nbytes.text_blocks = text_blocks_tmp.length();

            nbytes_text_blocks = statistics.nbytes.text_blocks;

            text_blocks = new uint8_t[nbytes_text_blocks];

            for (size_t i = 0; i < nbytes_text_blocks; i++)
                text_blocks[i] = text_blocks_tmp[i];

            assert(index == nlevels - 1);

            for (size_t i = 0; i < nlevels; i++) {
                statistics.nbytes.bit_vectors += level_bits[i]->size();
                statistics.nbytes.bit_vectors += level_runs[i]->size();

                statistics.nbytes.leaf_blocks += leaf_pointers[i]->size();
                statistics.nbytes.leaf_blocks += leaf_offsets[i]->size();
            }

            statistics.nbytes.total =
                sizeof nbytes_text +
                sizeof truncated_length +
                statistics.nbytes.bit_vectors +
                statistics.nbytes.leaf_blocks +
                sizeof nbytes_text_blocks +
                nbytes_text_blocks;

            break; // Nothing more to do here.
        }

        index++;

        level /= 2;

        curr_level.swap(next_level);

        next_level.clear();

    } // End while(level > 1).

    printf("nblocks.internal: %zu\n", statistics.nblocks.internal);
    printf("nblocks.leaf: %zu\n", statistics.nblocks.leaf);
    printf("nblocks.text: %zu\n", statistics.nblocks.text);

    printf("nbytes.input_text: %zu\n", nbytes_text);
    printf("nbytes.bit_vectors: %zu\n", statistics.nbytes.bit_vectors);
    printf("nbytes.leaf_blocks: %zu\n", statistics.nbytes.leaf_blocks);
    printf("nbytes.text_blocks: %zu\n", statistics.nbytes.text_blocks);
    printf("nbytes.total: %zu\n", statistics.nbytes.total);
}
