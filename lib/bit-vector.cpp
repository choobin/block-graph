#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bit-vector.h"
#include "utility.h"

BitVector::BitVector(size_t _nbits) :
    nbits(_nbits),
    nuints((_nbits + 64 - 1) / 64)
{
    data = new uint64_t[nuints];

    for (size_t i = 0; i < nuints; i++)
        data[i] = 0;
}

BitVector::BitVector(FILE *fp)
{
    assert(fp);

    Read(&nbits, sizeof nbits, 1, fp);

    nuints = (nbits + 64 - 1) / 64;

    data = new uint64_t[nuints];

    Read(data, sizeof(*data), nuints, fp);
}

BitVector::~BitVector()
{
    delete[] data;
}

void BitVector::save(FILE *fp) const
{
    assert(fp);

    Write(&nbits, sizeof nbits, 1, fp);

    Write(data, sizeof(*data), nuints, fp);
}

size_t BitVector::size() const
{
    return sizeof(nbits) + (nuints * sizeof(*data));
}

bool BitVector::operator[](size_t index) const
{
    assert(index < nbits);

    return get(index);
}

bool BitVector::get(size_t index) const
{
    assert(index < nbits);

    return ((data[index / 64] >> (index % 64)) & 1);
}

void BitVector::set(size_t index)
{
    assert(index < nbits);

    data[index / 64] |= ((uint64_t)1 << (index % 64));
}
