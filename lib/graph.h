#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>

class Array;
class Rank;

class Graph
{
public:
    Graph(const char*, size_t);

    Graph(FILE*);

    ~Graph();

    void save() const;

    uint8_t extract(size_t) const;

    void extract(uint8_t*, size_t, size_t) const;

    uint8_t *extract(size_t, size_t) const;

private:
    Graph();
    Graph(const Graph&);
    Graph& operator=(const Graph&);

    void init();

    void build();

    const char *path;

    size_t nbytes_text;

    size_t truncated_length;

    size_t nlevels;

    Rank **level_bits;
    Rank **level_runs;

    Array **leaf_offsets;
    Array **leaf_pointers;

    uint8_t *text_blocks;

    size_t nbytes_text_blocks;

    uint8_t extract_r(size_t, size_t, size_t, size_t) const;

    void extract_r(uint8_t*, size_t, size_t, size_t, size_t, size_t) const;
};

#endif /* GRAPH_H */
