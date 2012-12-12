#ifndef RMQ_H
#define RMQ_H

#include <stdint.h>

#include "rmq-succinct.h"

class RMQ {
public:
    RMQ(uint32_t *_array, uint32_t _length) :
        array(_array),
        length(_length),
        pimpl(NULL) {
        assert(_array);
        if (length > 256) // See: rmq-succinct.cpp:187-192.
            pimpl = new RMQ_succinct((int32_t*)array, (int32_t)length);
    }

    ~RMQ() {
        if (pimpl) delete pimpl;
    }

    uint32_t query(uint32_t start, uint32_t end) {
        assert(start <= end);

        if (pimpl && end - start > 256) // See: rmq-succinct.cpp:187-192.
            return pimpl->query(start, end);

        uint32_t minimum = start;

        for (size_t i = start + 1; i <= end; i++) {
            if (array[i] < array[minimum])
                minimum = i;
        }

        return minimum;
    }

private:
    RMQ();
    RMQ(const RMQ&);
    RMQ& operator=(const RMQ&);

    uint32_t *array;

    uint32_t length;

    RMQ_succinct *pimpl;
};

#endif
