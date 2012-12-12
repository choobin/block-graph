#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "divsufsort.h"
#include "rmq.h"
#include "stopwatch.h"
#include "text.h"
#include "utility.h"

Text::Text(const char *path)
{
    assert(path);

    Stopwatch stopwatch("[building sa]");

    FILE *fp = fopen(path, "r");

    check(fp, "Error: Failed to open %s", path);

    fseek(fp, 0L, SEEK_END);

    n = ftell(fp);

    fseek(fp, 0L, SEEK_SET);

    text = new uint8_t[n + 1];

    size_t nbytes = fread(text, 1, n, fp);

    check(nbytes == n, "Error: Failed to read %zu bytes from %s", n, path);

    text[n] = 0;

    fclose(fp);

    sa = new uint32_t[n];

    int ret = divsufsort(text, (int32_t*)sa, (int32_t)n);

    check(ret == 0, "Error: divsufsort failed '%d'", ret);

    stopwatch.stop();

    printf("\n"); // Pretty print.

    stopwatch.start("[building rmq]");

    rmq = new RMQ(sa, n);

    stopwatch.stop();

    printf("\n"); // Pretty print.
}

Text::~Text()
{
    delete rmq;
    delete[] sa;
    delete[] text;
}

inline size_t Text::refine(size_t lhs,
                           size_t rhs,
                           size_t offset,
                           uint8_t character,
                           refine_bound bound) const
{
    assert(lhs <= rhs);

    while (lhs <= rhs) {
        size_t mid = lhs + ((rhs - lhs) >> 1);

        uint8_t mid_value = text[sa[mid] + offset];

        if (mid_value < character)
            lhs = mid + 1;
        else if (mid_value > character) {
            if (mid == 0)
                return ~((size_t)0);

            rhs = mid - 1;
        }
        else {
            if ((bound == refine_lhs && mid == lhs) ||
                (bound == refine_rhs && mid == rhs))
                return mid; // Left/right most occurrence of character found.

            size_t next_offset = (bound == refine_lhs) ? -1 : 1;

            uint8_t next_value = text[sa[mid + next_offset] + offset];

            if (next_value == mid_value) {
                if (bound == refine_lhs)
                   rhs = mid - 1;
                else // if (bound == refine_rhs)
                   lhs = mid + 1;
            }
            else
                return mid; // Left/right most occurrence of character found.
        }
    }

    return ~((size_t)0);
}

size_t Text::first_occurrence(size_t i, size_t j) const
{
    assert(i <= j);

    size_t lhs = 0;
    size_t rhs = n - 1;

    size_t offset = 0;

    while (i + offset <= j) {
        // If lhs == rhs we are searching for the suffix at i (not
        // what we want). We a searching for the left most occurrence
        // of the string i..j.
        if (lhs == rhs)
            break;

        if (lhs == rhs && text[sa[lhs] + offset] != text[i + offset])
            break;

        lhs = refine(lhs, rhs, offset, text[i + offset], refine_lhs);
        if (lhs == ~((size_t)0))
            break;

        rhs = refine(lhs, rhs, offset, text[i + offset], refine_rhs);
        if (rhs == ~((size_t)0))
            break;

        offset++;
    }

    if (i + offset == j + 1) {
        size_t position = sa[rmq->query(lhs, rhs)];

        // The block-graph can not be self referential.
        if (position >= i || position + offset > i)
            return i;

        return position;
    }

    return i;
}

void Text::extract(uint8_t *ptr, size_t i, size_t j) const
{
    assert(ptr);
    assert(i + (j - i) < n);

    memcpy(ptr, text + i, j - i + 1);
}

size_t Text::size() const
{
    return n;
}
