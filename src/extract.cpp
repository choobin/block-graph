#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "graph.h"
#include "utility.h"

#define USECS_PER_SEC 1000000

static double usrsys()
{
    struct rusage usage;
    double usertime;
    double systime;

    getrusage(RUSAGE_SELF, &usage);

    usertime =
        (double) usage.ru_utime.tv_sec +
        (double) usage.ru_utime.tv_usec / USECS_PER_SEC;

    systime =
        (double) usage.ru_stime.tv_sec +
        (double) usage.ru_stime.tv_usec / USECS_PER_SEC;

    return usertime + systime;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <block-graph> <extract-file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *fp = fopen(argv[1], "r");
    check(fp, "Error: Failed to open %s", argv[0]);

    Graph graph(fp);

    fclose(fp);

    fp = fopen(argv[2], "r");
    check(fp, "Error: Failed to open %s", argv[1]);

    size_t n;
    size_t length;
    char file[BUFSIZ];

    int ret = fscanf(fp,
                     "# number=%zu length=%zu file=%s\n",
                     &n, &length, file);

    check(ret == 3, "Error: Invalid LZ-end extract header format");

    printf("n=%zu length=%zu file=%s\n", n, length, file);

    size_t i;
    size_t j;

    double elapsed = 0.0;
    size_t count = 0;

    while (fscanf(fp, "%zu,%zu\n", &i, &j) == 2) {
        double start = usrsys();

        uint8_t *str = graph.extract(i, j);

        double finish = usrsys();

        elapsed += (finish - start);

        delete[] str;

        count++;
    }

    check(count == n, "Error: Invalid number of i,j pairs '%zu'", count);

    // See: lz77index/tests/extract.cpp:64.

    printf("mchars/s = %.4f\n", (n * length) / elapsed / 1000000);

    printf("Total num chars extracted = %zu\n", n * length);

    printf("Extract time = %.4f secs\n\n\n", elapsed);

    fclose(fp);

    return EXIT_SUCCESS;
}
