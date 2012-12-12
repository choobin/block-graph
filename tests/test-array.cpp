#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <vector>
using std::vector;

#include "array.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <nitems> <max_value>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    size_t nitems = atoi(argv[1]);

    size_t max_value = atoi(argv[2]);

    srand(time(NULL));

    vector<uint32_t> v;
    for (size_t i = 0; i < nitems; i++)
        v.push_back(rand() % max_value);

    Array a(v);

    for (size_t i = 0; i < nitems; i++) {
        size_t result = a[i];
        if (result != v[i])
            fprintf(stderr,
                    "a[%zu] (%zu) != v[%zu] (%u)\n",
                    i, result, i, v[i]);
    }

    FILE *fp = fopen("test", "wb");
    assert(fp);

    a.save(fp);

    fclose(fp);

    fp = fopen("test", "rb");
    assert(fp);

    Array b(fp);

    for (size_t i = 0; i < nitems; i++) {
        size_t result = b[i];
        if (result != v[i])
            fprintf(stderr,
                    "b[%zu] (%zu) != v[%zu] (%u)\n",
                    i, result, i, v[i]);
    }

    fclose(fp);

    unlink("test");

    return EXIT_SUCCESS;
}
