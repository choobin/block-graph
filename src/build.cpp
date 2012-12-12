#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "utility.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <path> <truncated_depth>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *ptr;

    size_t truncated_depth = strtol(argv[2], &ptr, 10);

    check(*ptr == '\0', "Error: Invalid truncated depth value '%s'", argv[2]);

    Graph graph(argv[1], truncated_depth);

    graph.save();

    return EXIT_SUCCESS;
}
