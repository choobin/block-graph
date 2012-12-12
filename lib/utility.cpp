#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utility.h"

size_t next_pow2(size_t value)
{
    assert(value > 0);
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
#if __SIZEOF_SIZE_T__ == 8
    value |= value >> 32;
#endif
    value++;
    return value;
}

size_t log2_size_t(size_t v) {
    size_t r = 0;
#if __SIZEOF_SIZE_T__ == 8
    if (v & 0xFFFFFFFF00000000) { v >>= 32; r |= 32; }
#endif
    if (v & 0xFFFF0000)         { v >>= 16; r |= 16; }
    if (v & 0xFF00)             { v >>=  8; r |=  8; }
    if (v & 0xF0)               { v >>=  4; r |=  4; }
    if (v & 0xC)                { v >>=  2; r |=  2; }
    if (v & 0x2)                {           r |=  1; }
    return r;
}

bool is_file(const char *path)
{
    struct stat buffer;
    char actual_path[BUFSIZ];
    ssize_t ret;

    assert(path);

    ret = stat(path, &buffer);
    if (ret == -1)
        return false;

    if (S_ISLNK(buffer.st_mode)) {
        ret = readlink(path, actual_path, BUFSIZ);
        assert(ret != -1);

        ret = stat(actual_path, &buffer);
        assert(ret != -1);
    }

    return S_ISREG(buffer.st_mode);
}

size_t file_size(const char *path)
{
    FILE *fp;
    size_t nbytes;

    assert(path);

    check(is_file(path), "Error: %s is not a file", path);

    fp = fopen(path, "r");
    assert(fp);

    fseek(fp, 0L, SEEK_END);

    nbytes = ftell(fp);

    fclose(fp);

    return nbytes;
}
