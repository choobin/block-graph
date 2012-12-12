#ifndef ARRAY_H
#define ARRAY_H

#include <vector>

class Array
{
public:
    Array(const std::vector<uint32_t>&);

    Array(FILE*);

    ~Array();

    size_t operator[](size_t) const;

    size_t get(size_t) const;

    void set(size_t, size_t);

    void save(FILE*) const;

    size_t size() const;

private:
    Array();
    Array(const Array&);
    Array& operator=(const Array&);

    uint32_t *data;

    size_t nitems;

    size_t item_bits;

    size_t nuints;
};

#endif /* ARRAY_H */

