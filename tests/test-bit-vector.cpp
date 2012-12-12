#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <vector>
using std::vector;

#include "bit-vector.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <nbits>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    size_t nbits = atoi(argv[1]);

    srand(time(NULL));

    vector<bool> v;

    BitVector a(nbits);

    for (size_t i = 0; i < nbits; i++) {
        if (rand() < RAND_MAX / 2) {
            v.push_back(true);
            a.set(i);
        }
        else {
            v.push_back(false);
        }
    }

    for (size_t i = 0; i < nbits; i++) {
        bool result = a[i];
        if (result != v[i])
            fprintf(stderr,
                    "a[%zu] (%d) != v[%zu] (%d)\n",
                    i, result, i, (int)v[i]);
    }

    FILE *fp = fopen("test", "wb");
    assert(fp);

    a.save(fp);

    fclose(fp);

    fp = fopen("test", "rb");
    assert(fp);

    BitVector b(fp);

    for (size_t i = 0; i < nbits; i++) {
        bool result = b[i];
        if (result != v[i])
            fprintf(stderr,
                    "b[%zu] (%d) != v[%zu] (%d)\n",
                    i, result, i, (int)v[i]);
    }

    fclose(fp);

    unlink("test");

    return EXIT_SUCCESS;
}
