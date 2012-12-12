#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "utility.h"

static void
test_extract_i(Graph& graph, const uint8_t *text, size_t n)
{
    printf("\n%s -- ", __func__);
    fflush(stdout);

    for (size_t i = 0; i < n; i++) {
        uint8_t c = graph.extract(i);
        check(c == text[i],
              "i: %zu, c: %c != text[%zu]: %c",
              i, c, i, text[i]);
    }

    printf("passed\n");
}

static void
test_prefixes(Graph& graph, const uint8_t *text, size_t n, uint8_t *result)
{
    printf("* %s -- ", __func__);
    fflush(stdout);

    for (size_t i = 0; i < n; i++) {
        graph.extract(result, 0, i);

        check(memcmp(result, text, i) == 0,
              "result != text[%d..%zu]",
              0, i);

        check(result[i + 1] == 0,
              "Error: \\0 missing at result[%zu]",
              i + 1);
    }

    printf("passed\n");
}

static void
test_suffixes(Graph& graph, const uint8_t *text, size_t n, uint8_t *result)
{
    printf("* %s -- ", __func__);
    fflush(stdout);

    for (size_t i = n - 1; i > 0; i--) {
        graph.extract(result, i, n - 1);

        check(memcmp(result, text + i,n - i) == 0,
              "result != text[%zu..%zu]",
              i, n - 1);

        check(result[n - i] == 0,
              "Error: \\0 missing at result[%zu]",
              n - i);
    }

    graph.extract(result, 0, n - 1);

    check(memcmp(result, text, n - 1) == 0,
          "result != text[%d..%zu]",
          0, n - 1);

    check(result[n] == 0,
          "Error: \\0 missing at result[%zu]",
          n);

    printf("passed\n");
}

static void
test_ngrams(Graph& graph, const uint8_t *text, size_t n, uint8_t *result)
{
    printf("* %s -- ", __func__);
    fflush(stdout);

    for (size_t ngram_size = 2;
         ngram_size < n;
         ngram_size++) {

        for (size_t i = 0;
             i < n - ngram_size + 1;
             i++) {

            graph.extract(result, i, i + (ngram_size - 1));

            check(memcmp(result, text + i, ngram_size) == 0,
                  "result: %s != text[%zu..%zu]",
                  result, i, i + (ngram_size + 1));

            check(result[ngram_size] == 0,
                  "Error: \\0 missing at result[%zu]",
                  ngram_size);
        }
    }

    printf("passed\n");
}

static void
test_extract_ij(Graph& graph, const uint8_t *text, size_t n)
{
    printf("\n%s ((this could take a while))\n", __func__);

    uint8_t *result = new uint8_t[n + 1];

    test_prefixes(graph, text, n, result);

    test_suffixes(graph, text, n, result);

    test_ngrams(graph, text, n, result);

    delete[] result;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <path> <truncated_depth>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Graph graph(argv[1], atoi(argv[2]));

    printf("\nLoading: %s\n", argv[1]);

    size_t n = file_size(argv[1]);

    FILE *fp = fopen(argv[1], "r");
    check(fp, "Error: Could not load input file '%s'", argv[1]);

    uint8_t *text = new uint8_t[n + 1];

    check(fread(text, 1, n, fp) == n,
          "Error: Failed to read %zu bytes from input file", n);

    text[n] = 0;

    fclose(fp);

    test_extract_i(graph, text, n);

    test_extract_ij(graph, text, n);

    delete[] text;

    return EXIT_SUCCESS;
}
