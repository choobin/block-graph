#ifndef TEXT_H
#define TEXT_H

class RMQ;

class Text {
public:
    Text(const char*);

    ~Text();

    size_t first_occurrence(size_t, size_t) const;

    void extract(uint8_t*, size_t, size_t) const;

    size_t size() const;

private:
    Text();
    Text(const Text&);
    Text& operator=(const Text&);

    uint8_t *text;

    uint32_t *sa;

    size_t n;

    RMQ *rmq;

    enum refine_bound {
        refine_lhs,
        refine_rhs
    };
    size_t refine(size_t, size_t, size_t, uint8_t, refine_bound) const;
};

#endif /* TEXT_H */
