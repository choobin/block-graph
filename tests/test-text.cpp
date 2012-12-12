#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "text.h"
#include "utility.h"

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr,
                "usage: %s <text> <ntests> <extract length>\n",
                argv[0]);

        exit(EXIT_FAILURE);
    }

    Text text(argv[1]);

    FILE *fp = fopen(argv[1], "r");
    check(fp, "Error: Failed to open %s", argv[0]);

    fseek(fp, 0L, SEEK_END);

    size_t n = ftell(fp);

    fseek(fp, 0L, SEEK_SET);

    uint8_t *raw_text = new uint8_t[n + 1];

    size_t nbytes = fread(raw_text, 1, n, fp);

    check(nbytes == n, "Error: Failed to read %zu bytes from %s", n, argv[1]);

    raw_text[n] = 0;

    fclose(fp);

    srand(time(NULL));

    size_t ntests = atoi(argv[2]);

    size_t position;

    size_t length = atoi(argv[3]);

    size_t left_most;

    size_t count = 0;

    for (size_t i = 0; i < ntests; i++) {
        position = (size_t)(((double)rand() / RAND_MAX) * (n - length));

        left_most = text.first_occurrence(position, position + length - 1);

        if (left_most < position) {
            check(memcmp(raw_text + position,
                         raw_text + left_most, length) == 0,
                  "Error: lm: %zu != pos %zu", left_most, position);

            uint8_t str[length + 1];

            memcpy(str, raw_text + position, length);

            str[length] = 0;

            // Search from the beginning of the text :D.
            uint8_t *ptr = (uint8_t*)strstr((char*)raw_text, (char*)str);

            size_t strstr_position = ptr - raw_text;

            check(left_most == strstr_position,
                  "Error: text.first_occurrence(pos, len): %zu "
                  "but strstr returned: %zu",
                  left_most,
                  strstr_position);

            count++;
        }
    }

    printf("count: %zu\n", count);

    delete[] raw_text;

    return EXIT_SUCCESS;
}
