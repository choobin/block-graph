#ifndef UTILITY_H
#define UTILITY_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined NDEBUG
#define debug(message, ...)                                             \
    fprintf(stderr,                                                     \
            message "\n",                                               \
            ##__VA_ARGS__)
#else
#define debug(message, ...)                                             \
    fprintf(stderr,                                                     \
            "%s:%s:%d " message "\n",                                   \
            __FILE__,                                                   \
            __func__,                                                   \
            __LINE__,                                                   \
            ##__VA_ARGS__)
#endif

#define check(predicate, message, ...)                                     \
    if (!(predicate)) { debug(message, ##__VA_ARGS__); exit(EXIT_FAILURE); }

#define ___IO(operation, value, size, n, fp) do {                           \
        size_t __nitems = f##operation(value, size, n, fp);                 \
        check(__nitems == n, "f" #operation " error: %s", strerror(errno)); \
    } while(0)

#define Read(value, size, n, fp) ___IO(read, value, size, n, fp)

#define Write(value, size, n, fp) ___IO(write, value, size, n, fp)

size_t next_pow2(size_t);

size_t log2_size_t(size_t);

bool is_file(const char*);

size_t file_size(const char*);

#endif /* UTILITY_H */
