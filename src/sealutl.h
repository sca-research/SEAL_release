#ifndef _SEALUTL_H
#define _SEALUTL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

    char *CloneString(const char *str);
    void *Malloc(size_t size);
    void Free(const void *p);
    void Close(int fd);

#ifdef __cplusplus
}
#endif
#endif
