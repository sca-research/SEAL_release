#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "sealconfig.h"

#include "sealutl.h"

//Allocate memory and copy a string.
char *CloneString(const char *str)
{
    char *s = NULL;

    //Use strdup if supported.
#if _XOPEN_SOURCE >= 500 || _POSIX_C_SOURCE >= 200809L || _BSD_SOURCE || _SVID_SOURCE
    s = strndup(str, SEAL_CORE_MAXSTRLEN);
#else
    //Use customised implementation.
    size_t slen = 0;

    slen = strnlen(str, SEAL_CORE_MAXSTRLEN);
    s = (char *)malloc(slen + 1);
    memset(s, 0, slen + 1);
    strncpy(s, str, slen);
#endif
    return s;
}

void *Malloc(size_t size)
{
    void *addr = NULL;
    addr = malloc(size);
    memset(addr, 0, size);
    return addr;
}

//General destructors().
void Free(const void *p)
{
    if (NULL != p)
	{
	    free((void *)p);
	}
    return;
}

void Close(int fd)
{
    if (fd >= 0)
	{
	    close(fd);
	}
    return;
}

//Count number of JSON objects under ${cjsonroot}.
