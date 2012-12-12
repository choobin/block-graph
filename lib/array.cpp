#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>
using std::vector;

#include "array.h"
#include "utility.h"

Array::Array(const vector<uint32_t>& array)
{
    data = NULL;

    nitems = array.size();

    if (nitems == 0)
        return;

    size_t max_value = array[0];

    for (size_t i = 1; i < nitems; i++) {
        if (max_value < array[i])
            max_value = array[i];
    }

    // A nasty little corner case.
    if (max_value == 0)
        max_value = 1;

    item_bits = 0;

    while (max_value) {
        item_bits++;
        max_value >>= 1;
    }

    size_t nbits = nitems * item_bits;

    nuints = (nbits + 32 - 1) / 32;

    data = new uint32_t[nuints];

    for (size_t i = 0; i < nuints; i++)
        data[i] = 0;

    for (size_t i = 0; i < nitems; i++)
        set(i, array[i]);
}

Array::Array(FILE *fp)
{
    assert(fp);

    data = NULL;

    Read(&nitems, sizeof nitems, 1, fp);

    if (nitems == 0)
        return;

    Read(&item_bits, sizeof item_bits, 1, fp);

    size_t nbits = nitems * item_bits;

    nuints = (nbits + 32 - 1) / 32;

    data = new uint32_t[nuints];

    Read(data, sizeof(*data), nuints, fp);
}

Array::~Array()
{
    if (data) delete[] data;
}

void Array::save(FILE *fp) const
{
    assert(fp);

    Write(&nitems, sizeof nitems, 1, fp);

    if (nitems == 0)
        return;

    Write(&item_bits, sizeof item_bits, 1, fp);

    Write(data, sizeof(*data), nuints, fp);
}

size_t Array::size() const
{
    if (nitems == 0)
        return sizeof(nitems);

    size_t nbytes =
        sizeof(nitems) +
        sizeof(item_bits) +
        (nuints * sizeof(*data));

    return nbytes;
}

size_t Array::operator[](const size_t index) const
{
    return get(index);
}

size_t Array::get(size_t index) const
{
    assert(index < nitems);

    size_t i = index * item_bits / 32;
    size_t j = index * item_bits - 32 * i;

    size_t result;

    if (j + item_bits <= 32) {
        result = (data[i] << (32 - j - item_bits)) >> (32 - item_bits);
    }
    else {
        result = data[i] >> j;
        result |= (data[i + 1] << (64 - j - item_bits)) >> (32 - item_bits);
    }

    return result;
}

void Array::set(size_t index, size_t value)
{
    assert(index < nitems);

    size_t i = index * item_bits / 32;
    size_t j = index * item_bits - 32 * i;

    uint32_t mask = ~((uint32_t)0);

    if (j + item_bits < 32)
        mask = ~((uint32_t)0) << (j + item_bits);

    if (32 - j < 32)
        mask |= ~((uint32_t)0) >> (32 - j);

    data[i] = (data[i] & mask) | value << j;

    if (j + item_bits > 32) {
        mask = ((~((uint32_t)0)) << (item_bits + j - 32));
        data[i + 1] = (data[i + 1] & mask) | value >> (32 - j);
    }
}
