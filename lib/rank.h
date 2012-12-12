#ifndef RANK_H
#define RANK_H

#include <assert.h>

#include "bit-vector.h"
#include "rank9b.h"

class Rank
{
public:
    Rank(BitVector* _bv) :
        bv(_bv),
        pimpl(new rank9b(bv->data, bv->nbits)) {
    }

    Rank(FILE *fp) :
        bv(new BitVector(fp)),
        pimpl(new rank9b(bv->data, bv->nbits)) {
    }

    ~Rank() {
        delete bv;
        delete pimpl;
    }

    bool access(size_t index) const {
        return bv->get(index);
    }

    size_t rank1(size_t index) const {
        return pimpl->rank(index);
    }

    size_t rank0(size_t index) const {
        return index - pimpl->rank(index);
    }

    void save(FILE *fp) const {
        bv->save(fp);
    }

    size_t size() const {
        return bv->size();
    }

private:
    Rank();
    Rank(const Rank&);
    Rank& operator=(const Rank&);

    BitVector *bv;
    rank9b *pimpl;
};

#endif /* RANK_H */
