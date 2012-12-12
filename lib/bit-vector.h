#ifndef BIT_VECTOR_H
#define BIT_VECTOR_H

#include <stdint.h>
#include <stdio.h>

class BitVector
{
public:
    BitVector(size_t);

    BitVector(FILE*);

    ~BitVector();

    bool operator[](size_t) const;

    bool get(size_t) const;

    void set(size_t);

    void save(FILE*) const;

    size_t size() const;

private:
    BitVector();
    BitVector(const BitVector&);
    BitVector& operator=(const BitVector&);

    uint64_t *data;

    size_t nbits;

    size_t nuints;

    friend class Rank;
};

#endif /* BIT_VECTOR_H */
