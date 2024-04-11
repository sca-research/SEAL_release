#ifndef _TRACE_H
#define _TRACE_H

#include "core.h"
#include "frame.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SEAL_TRACE_MODE_READ (0x0)
#define SEAL_TRACE_MODE_CREATE (0x1)
#define SEAL_TRACE_MODE_APPEND (0x2)
#define SEAL_TRACE_MODE_FIFO (0x3)
#define SEAL_TRACE_FLAG_DEFAULT (0x0)

    //Seal Trace.
    typedef struct _SealTrace_ {
	const SealCore *core;
	char *path;
	int fd;
	int mode;
	size_t offset;
	int flag;		//Reserved
	void **args;		//Reserved
    } SealTrace;

    //Constructor/Destructor for Seal Trace.
    SealTrace *StOpen(const char *path, const SealCore * core, const int mode);
    void StClose(SealTrace * trace);
    //Add a frame into the Trace.
    int StAddFrame(SealTrace * trace, SealFrame * frame);
    //Read the next frame from the trace.
    int StNextFrame(SealTrace * trace, SealFrame * frame);
#ifdef __cplusplus
}
#endif
#endif
