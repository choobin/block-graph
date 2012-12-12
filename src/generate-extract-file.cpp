#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utility.h"

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "usage: %s <input> <n> <length>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *fp = fopen(argv[1], "r");
    check(fp, "Error: Could not open input file '%s'", argv[1]);

    int ret = fseek(fp, 0L, SEEK_END);
    check(ret == 0, "Error: Failed to seek to end of file '%d'", ret);

    long nbytes = ftell(fp);
    check(nbytes != -1, "Error: Failed to seek to end of file");

    fclose(fp);

    char *ptr;

    size_t n = strtoll(argv[2], &ptr, 10);
    check(*ptr == '\0', "Error: Invalid n value '%s'", argv[2]);

    size_t length = strtoll(argv[3], &ptr, 10);
    check(*ptr == '\0', "Error: Invalid length value '%s'", argv[3]);

    // LZ-end extract input file requires a specific header format.
    // See: lz77index/tests/extract.cpp:45.

    printf("# number=%zu length=%zu file=%s\n", n, length, argv[1]);

    for (size_t i = 0; i < n; i++) {
        size_t lhs = (size_t)(drand48() * (nbytes - length));
        size_t rhs = lhs + length - 1;
        printf("%zu,%zu\n", lhs, rhs);
    }

    return EXIT_SUCCESS;
}
