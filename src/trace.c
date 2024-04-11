#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "seal.h"
#include "sealutl.h"
#include "core.h"
#include "trace.h"
#include "version.h"

inline bool StIsValid(const int mode, const int flag)
{
    //Check ${mode}.
    switch (mode)
	{
	    //Valid modes.
	case SEAL_TRACE_MODE_READ:
	case SEAL_TRACE_MODE_CREATE:
	case SEAL_TRACE_MODE_APPEND:
	case SEAL_TRACE_MODE_FIFO:
	    return true;
	    break;

	    //By default invalid modes.
	default:
	    return false;
	    break;
	}

    //Check ${flag}.
    if (SEAL_TRACE_FLAG_DEFAULT == flag)
	return true;
//Disable interactive trace.
#if 0
    if (SEAL_TRACE_FLAG_INTERACTIVE & flag)
	{
	    //The Interacitve flag is set.
	    if (SEAL_TRACE_MODE_READ != mode)
		{
		    INFO("#SEAL_TRACE_FLAG_INTERACTIVE must be used with SEAL_TRACE_MODE_READ.\n");
		    return false;
		}
	}
#endif
    return true;
}

//Constructor/Destroctor of a Seal Trace.
SealTrace *StOpen(const char *path, const SealCore * core, const int mode)
{
    SealTrace *trace = NULL;
    int oflag;			//Flag for UNIX open().
    static const int MODE_RW_ALL = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);	//Default mode allowing all to read and write.

    //Initialise a trace.
    trace = (SealTrace *) Malloc(sizeof(SealTrace));
    trace->path = CloneString(path);
    trace->core = core;
    trace->fd = -1;
    trace->mode = mode;
    trace->offset = 0;
    trace->flag = SEAL_TRACE_FLAG_DEFAULT;
    trace->args = NULL;

    //Check validity of ${mode} and ${flag}.
    if (!StIsValid(trace->mode, trace->flag))
	goto error;

    //Set up file mode.
    switch (mode)
	{
	case SEAL_TRACE_MODE_READ:
	    oflag = O_RDONLY;
	    break;
	case SEAL_TRACE_MODE_CREATE:
	    oflag = O_WRONLY | O_TRUNC | O_CREAT;
	    break;
	case SEAL_TRACE_MODE_APPEND:
	    oflag = O_WRONLY | O_APPEND;
	    break;
	case SEAL_TRACE_MODE_FIFO:
	    mkfifo(path, 0666);	//Enable read and write for all.
	    oflag = O_WRONLY | O_TRUNC;
	    break;
	default:
	    INFO("#Unknown Trace mode.");
	    goto error;
	    break;
	}

    //Open file.
    if (-1 == (trace->fd = open(trace->path, oflag, MODE_RW_ALL)))
	{
	    INFO("#open() failed:");
	    INFO(strerror(errno));
	    goto error;
	}

    return trace;

 error:
    if (mode == SEAL_TRACE_MODE_FIFO)
	{
	    unlink(path);
	}
    Free(trace->path);
    Free(trace);
    INFO("#Failed to open Trace.\n");
    return NULL;
}

void StClose(SealTrace * trace)
{
    Close(trace->fd);
    if (SEAL_TRACE_MODE_FIFO == trace->mode)
	{
	    unlink(trace->path);
	}

    Free(trace->path);
    Free(trace);
    return;
}

//Add a Frame ${frame} into Trace ${trace}.
int StAddFrame(SealTrace * trace, SealFrame * frame)
{
    int nwrite = 0;

    //Validity check.
    if (trace->core != frame->core)
	{
	    INFO("#Unmatched Core.\n");
	    return SEAL_ERROR;
	}

    if ((trace->mode == SEAL_TRACE_MODE_CREATE) &&
	(trace->mode == SEAL_TRACE_MODE_APPEND) &&
	(trace->mode == SEAL_TRACE_MODE_FIFO))
	{
	    INFO("#Trace not opened for write.\n");
	    return SEAL_ERROR;
	}

    //Write Frame into Trace file.
    if (-1 == (nwrite = write(trace->fd, frame->buf, frame->len)))
	{
	    INFO("#write() failed:");
	    INFO(strerror(errno));
	    return SEAL_ERROR;
	}
    trace->offset++;
    return nwrite;
}

//Get the next Frame in Trace ${trace} into ${frame}.
int StNextFrame(SealTrace * trace, SealFrame * frame)
{
    int nread = -1;

    //Validity check.
    if (trace->core != frame->core)
	{
	    INFO("#Unmatched Core.\n");
	    return SEAL_ERROR;
	}

    if (trace->mode != SEAL_TRACE_MODE_READ)
	{
	    INFO("#Trace not opened for read.\n");
	    return SEAL_ERROR;
	}

    //Read Frames from  Trace file into buffer.
    if (-1 == (nread = read(trace->fd, frame->buf, frame->len)))
	{
	    INFO("#read() failed:");
	    INFO(strerror(errno));
	    return SEAL_ERROR;
	}

    if (0 == nread)
	{
	    INFO("#Trace EOF.\n");
	    return 0;
	}
    else if (frame->len > nread)
	{			//Less than frame length is read.
	    INFO("#Corrupted Frame.\n");
	    return SEAL_ERROR;
	}

    trace->offset++;

    return nread;
}
