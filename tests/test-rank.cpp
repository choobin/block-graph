#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <vector>
using std::vector;

#include "bit-vector.h"
#include "rank.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <nbits>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    size_t nbits = atoi(argv[1]);

    srand(time(NULL));

    vector<size_t> v0;
    vector<size_t> v1;

    size_t zero_count = 0;
    size_t one_count = 0;

    v0.push_back(zero_count);
    v1.push_back(one_count);

    BitVector *a = new BitVector(nbits);

    for (size_t i = 0; i < nbits; i++) {
        if (rand() < RAND_MAX / 2) {
            one_count++;
            a->set(i);
        }
        else
            zero_count++;

        v1.push_back(one_count);
        v0.push_back(zero_count);
    }

    Rank r(a);

    for (size_t i = 0; i < nbits; i++) {
        size_t result = r.rank1(i);
        if (result != v1[i])
            fprintf(stderr,
                    "rank1 a[%zu] (%zu) != v[%zu] (%zu)\n",
                    i, result, i, v1[i]);
    }

    for (size_t i = 0; i < nbits; i++) {
        size_t result = r.rank0(i);
        if (result != v0[i])
            fprintf(stderr,
                    "rank0 a[%zu] (%zu) != v[%zu] (%zu)\n",
                    i, result, i, v0[i]);
    }

    return EXIT_SUCCESS;
}
