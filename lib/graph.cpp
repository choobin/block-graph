#include <assert.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "bit-vector.h"
#include "graph.h"
#include "rank.h"
#include "utility.h"

Graph::Graph(const char *_path, size_t _truncated_length) :
    path(_path),
    truncated_length(_truncated_length)
{
    assert(path);

    check(is_file(path), "Error: %s is not a file", path);

    check(truncated_length >= 2,
          "Error: truncated_length '%zu' must be >= 2",
          truncated_length);

    check(next_pow2(truncated_length) == truncated_length,
          "Error: truncated_length '%zu' must be a power of two",
          truncated_length);

    nbytes_text = file_size(path);

    nlevels =
        log2_size_t(next_pow2(nbytes_text)) -
        (log2_size_t(truncated_length) - 1);

    init();

    build();
}

Graph::Graph(FILE *fp)
{
    assert(fp);

    path = NULL;

    Read(&nbytes_text, sizeof nbytes_text, 1, fp);

    Read(&truncated_length, sizeof truncated_length, 1, fp);

    nlevels =
        log2_size_t(next_pow2(nbytes_text)) -
        (log2_size_t(truncated_length) - 1);

    init();

    for (size_t i = 0; i < nlevels; i++) {
        level_bits[i] = new Rank(fp);
        level_runs[i] = new Rank(fp);

        leaf_pointers[i] = new Array(fp);
        leaf_offsets[i] = new Array(fp);
    }

    Read(&nbytes_text_blocks, sizeof nbytes_text_blocks, 1, fp);

    text_blocks = new uint8_t[nbytes_text_blocks];

    Read(text_blocks, nbytes_text_blocks, 1, fp);
}

void Graph::init()
{
    level_bits = new Rank*[nlevels];
    level_runs = new Rank*[nlevels];

    leaf_pointers = new Array*[nlevels];
    leaf_offsets = new Array*[nlevels];

    for (size_t i = 0; i < nlevels; i++) {
        level_bits[i] = NULL;
        level_runs[i] = NULL;
        leaf_pointers[i] = NULL;
        leaf_offsets[i] = NULL;
    }

    text_blocks = NULL;
}

Graph::~Graph()
{
    for (size_t i = 0; i < nlevels; i++) {
        delete level_bits[i];
        delete level_runs[i];
        delete leaf_pointers[i];
        delete leaf_offsets[i];
    }
    delete[] level_bits;
    delete[] level_runs;
    delete[] leaf_pointers;
    delete[] leaf_offsets;

    delete[] text_blocks;
}

void Graph::save() const
{
    char *copy = strdup(path);

    assert(copy);

    char *base = basename(copy);

    char buf[BUFSIZ];
    snprintf(buf, BUFSIZ, "%s.block-graph", base);

    free(copy);

    FILE *fp = fopen(buf, "w");
    check(fp, "Error: Failed to open '%s'", buf);

    Write(&nbytes_text, sizeof nbytes_text, 1, fp);

    Write(&truncated_length, sizeof truncated_length, 1, fp);

    for (size_t i = 0; i < nlevels; i++) {
        level_bits[i]->save(fp);
        level_runs[i]->save(fp);
        leaf_pointers[i]->save(fp);
        leaf_offsets[i]->save(fp);
    }

    Write(&nbytes_text_blocks, sizeof nbytes_text_blocks, 1, fp);

    Write(text_blocks, nbytes_text_blocks, 1, fp);

    fclose(fp);
}
