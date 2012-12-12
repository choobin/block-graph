#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "graph.h"
#include "stopwatch.h"
#include "utility.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <path> <truncated_depth>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Graph graph(argv[1], atoi(argv[2]));

    printf("\nLoading: %s\n", argv[optind]);

    size_t n = file_size(argv[1]);

    FILE *fp = fopen(argv[1], "r");

    check(fp, "Error: Could not load input file '%s'", argv[1]);

    uint8_t *text = new uint8_t[n + 1];

    check(fread(text, 1, n, fp) == n,
          "Error: Failed to read %zu bytes from input file", n);

    text[n] = '\0';

    fclose(fp);

    Stopwatch stopwatch("Testing extract(i)");

    for (size_t k = 0; k < n; k++) {
        uint8_t c = graph.extract(k);
        check(c == text[k],
              "graph.extract(%zu) failed: '%c' %d != text[%zu] '%c'\n",
              k, c, c, k, text[k]);
    }

    stopwatch.stop("Passed");

    printf("\n"); // Pretty print.

    uint8_t *result = new uint8_t[n + 1];

    stopwatch.start("Testing extract(0, n - 1)");

    graph.extract(result, 0, n - 1);

    for (size_t k = 0; k < n; k++) {
        check(result[k] == text[k],
              "Error: result[%zu] (%c) != text[%zu + %zu] (%c)\n",
              k, result[k], k, k, text[k]);
    }

    stopwatch.stop("Passed");

    delete[] result;

    delete[] text;

    return EXIT_SUCCESS;
}
