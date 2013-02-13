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

static void test(size_t nbits, double density)
{
    srand48(time(NULL));

    vector<size_t> rank0;
    vector<size_t> rank1;

    size_t rank0_count = 0;
    size_t rank1_count = 0;

    rank0.push_back(rank0_count);
    rank1.push_back(rank1_count);

    BitVector *a = new BitVector(nbits);

    for (size_t i = 0; i < nbits; i++) {
        if (drand48() < density) {
            rank1_count++;
            a->set(i);
        }
        else {
            rank0_count++;
        }

        rank1.push_back(rank1_count);
        rank0.push_back(rank0_count);
    }

    Rank r(a);

    for (size_t i = 0; i < nbits; i++) {
        size_t result = r.rank1(i);
        if (result != rank1[i]) {
            fprintf(stderr,
                    "rank1 a[%zu] (%zu) != v[%zu] (%zu)\n",
                    i, result, i, rank1[i]);

            assert(result != rank1[i]);
        }
    }

    for (size_t i = 0; i < nbits; i++) {
        size_t result = r.rank0(i);
        if (result != rank0[i]) {
            fprintf(stderr,
                    "rank0 a[%zu] (%zu) != v[%zu] (%zu)\n",
                    i, result, i, rank0[i]);

            assert(result != rank0[i]);
        }
    }
}

int main()
{
    size_t test_nbits[] = {1ULL << 20, 2ULL << 20, 4ULL << 20, 10ULL << 20, 20ULL << 20};
    double test_densities[] = {0.1, 0.2, 0.4, 0.8};
    size_t i;
    size_t j;

    for (i = 0; i < sizeof test_nbits / sizeof test_nbits[0]; i++) {
        for (j = 0; j < sizeof test_densities / sizeof test_densities[0]; j++) {
            printf("nbits: %lu, density: %.2f\n",
                   (unsigned long)test_nbits[i],
                   test_densities[j]);

            test(test_nbits[i], test_densities[j]);
        }
    }

    return EXIT_SUCCESS;
}
