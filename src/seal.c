#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "sealconfig.h"
#include "sealutl.h"
#include "core.h"
#include "trace.h"
#include "frame.h"
#include "symbolic.h"

#include "seal.h"

bool sealinfo = false;

void SealSetVerbose(bool newsealinfo)
{
    sealinfo = newsealinfo;
    return;
}

//Print info.
void INFO(const char *errmsg)
{
#if SEALINFO
    if (sealinfo)
	{
	    fprintf(stderr, errmsg, "");
	}
#endif
    return;
}

//Seal Errno.
int SealErrno()
{
    switch (errno)
	{
	case 0:
	    return SEAL_SUCCESS;
	case EEXIST:
	    return SEAL_EXIST;
	default:
	    return SEAL_ERROR;
	}
}

//Simplifed APIs.
//Delete a chain of MapVal.
static void DelMap(ValMap * head)
{
    ValMap *current = head;
    ValMap *next;

    while (NULL != current)
	{
	    next = current->next;
	    //Free the current MapVal.
	    free(current->ci);
	    free(current);
	    current = next;
	}

    return;
}

//Initialise a SEAL object.
Seal *InitSeal(const char *corepath, const char *tracepath, int tracemode)
{
    Seal *seal = NULL;

    seal = (Seal *) malloc(sizeof(Seal));
    seal->core = ScLoadCore(corepath);
    seal->trace = StOpen(tracepath, seal->core, tracemode);
    seal->frame = SfNewFrame(seal->core);
    seal->maps = NULL;
    seal->dict = NewDict();
    seal->eventlog = NewEventLog();

    return seal;
}

//Free a SEAL object.
void FreeSeal(Seal * seal)
{
    DelEventLog(seal->eventlog);
    DelDict(seal->dict);
    DelMap(seal->maps);
    SfDeleteFrame(seal->frame);
    StClose(seal->trace);
    ScDeleteCore(seal->core);
    free(seal);

    return;
}

//Retrive a hanlde of Component by its name, or NULL if no such Component found.
SealComponent *SealGetComponent(Seal * seal, const char *compname)
{
    int i;
    SealComponent **comps = seal->core->components;

    for (i = 0; i < seal->core->ncomponents; i++)
	{
	    if (0 == strncmp(comps[i]->name, compname, SEAL_CORE_MAXSTRLEN))
		{
		    return comps[i];
		}
	}

    //Component not found in Core.
    return NULL;
}

//Read next Frame into the buffer.
int SealNextFrame(Seal * seal)
{
    int ret = 0;

    //Read the current Frame in ${seal->trace} into the buffer ${seal->frame}.
    ret = StNextFrame(seal->trace, seal->frame);
    if (0 == ret)		//Trace reached EOF.
	{
	    return 0;
	}
    else if (0 > ret)		//Error reading Frame.
	{
	    return SEAL_ERROR;
	}

    //Returns the ID of the read Frame.
    return seal->trace->offset;
}

//Fetch the current Frame into the buffer. This API does not affect the Trace offet.
int SealFetchFrame(SealFrame * frame, Seal * seal, unsigned long frameno)
{
    int ret;
    const int tracefd = seal->trace->fd;	//FD for Trace file.
    const size_t framelen = seal->frame->len;	//Size of a Frame in bytes.
    size_t offset = 0;		//Offset in Trace.
    struct stat fs = { 0 };

    //Input validation.
    if (frame->core != seal->core)	//Unmatched Core.
	{
	    INFO("#Unmatched Core.\n");
	    return SEAL_ERROR;
	}

    if (frame->len != seal->frame->len)	//Unmatched Frame length.
	{
	    INFO("#Unmatched Frame len.\n");
	    return SEAL_ERROR;
	}

    if (-1 == fstat(tracefd, &fs))	//Wrong Trace file.
	{
	    perror("fstat() error:");
	    return SEAL_ERROR;
	}
    if (S_IFREG != (fs.st_mode & S_IFMT))
	{
	    INFO("#FetchFrame() can only be used on regular Trace files.\n");
	    return SEAL_ERROR;
	}

    if (0 == frameno)		//Wrong index.
	{
	    INFO("#frameno begins from 1.\n");
	    return SEAL_ERROR;
	}
    frameno--;

    //Compute the offset in the Trace.
    offset = frameno * framelen;

    //Read into ${frame}.
    ret = pread(tracefd, frame->buf, framelen, offset);
    if (0 == ret)
	{
	    INFO("#No such Frame.\n");
	    return 0;
	}
    if (0 > ret)
	{
	    perror("perror() failed:");
	    return SEAL_ERROR;
	}

    return ret;
}

//Create a new empty Frame.
SealFrame *SealNewFrame(Seal * seal)
{
    return SfNewFrame(seal->core);
}

//Copy a Frame from ${srcframe} to ${dstframe}.
int SealCopyFrame(SealFrame * dstframe, const SealFrame * srcframe)
{
    if (dstframe->core != srcframe->core)
	{
	    INFO("#Unmatched Core.\n");
	    return SEAL_ERROR;
	}

    if (dstframe->len != srcframe->len)
	{
	    INFO("#Unmatched Frame len.\n");
	    return SEAL_ERROR;
	}

    //Copy the contents from ${srcframe} to ${dstframe}.
    memcpy(dstframe->buf, srcframe->buf, dstframe->len);

    return SEAL_SUCCESS;
}

//Free ${frame}.
void SealFreeFrame(SealFrame * frame)
{
    SfDeleteFrame(frame);
    return;
}

//Fetch the Component from $frame$ by its name ${compname}.
int SealFetchComp(ComponentInstance * comp, SealFrame * frame,
		  const char *compname)
{
    int i;
    int validname = false;
    SealCoreComponent **corecomps = frame->core->components;	//Quick reference to Components in the Core.

    //Fetch Component meta information from Core.
    for (i = 0; i < frame->core->ncomponents; i++)
	{
	    if (0 == strncmp(corecomps[i]->name, compname, SEAL_CORE_MAXSTRLEN))
		{
		    validname = true;
		    break;
		}
	}
    if (!validname)
	{
	    INFO("#Invalid Component name.\n");
	    return SEAL_ERR_NO_SUCH_COMPONENT;
	}

    //Record corresponding Frame.
    comp->frame = frame;

    //Fetch Component Instance from the Frame.
    if (SEAL_SUCCESS != SfFetchFrameCI(frame, &comp->ci, compname))
	{
	    INFO("#Corrupted Frame.\n");
	    return SEAL_ERR_CORRUPTED_FRAME;
	}

    //Copy Component meta information.
    comp->version = frame->core->versionid;
    comp->name = frame->core->components[i]->name;
    comp->type = frame->core->components[i]->type;
    comp->typesize = SccSizeOf(comp->type);
    comp->typestr = SccGetTypeStr(comp->type);
    comp->size = frame->core->components[i]->len;

    //Set short cut for binary Component Instance.
    comp->val.raw = comp->ci.val;

    return SEAL_SUCCESS;
}

//List at ${complist} all Component Instances within a given Frame. Returns the number of Components on success.
int SealListComp(ComponentInstance ** complist, SealFrame * frame)
{
    int i;
    long offset;
    const int ncomp = frame->core->ncomponents;
    SealCoreComponent **corecomps = frame->core->components;	//Quick reference to Components in the Core.
    ComponentInstance *comp;

    *complist = (ComponentInstance *) Malloc(sizeof(ComponentInstance) * ncomp);

    //Fill in each Component Instance.
    for (i = 0, comp = *complist, offset = 0; i < ncomp; i++, comp++)
	{
	    //Fill meta information.
	    comp->name = corecomps[i]->name;
	    comp->type = corecomps[i]->type;
	    comp->typesize = SccSizeOf(comp->type);
	    comp->typestr = SccGetTypeStr(comp->type);
	    comp->size = frame->core->components[i]->len;

	    //Set value filed.
	    comp->val.raw = frame->body + offset;
	    //Update offset.
	    offset += corecomps[i]->len * SccSizeOf(corecomps[i]->type);
	}

    return ncomp;
}

//Write a SEAL frame into the trace.
int SealWrite(Seal * seal)
{
    return StAddFrame(seal->trace, seal->frame);
}

//Write a Frame into the Trace in ${seal}.
int SealWriteFrame(Seal * seal, SealFrame * frame)
{
    return StAddFrame(seal->trace, frame);
}

//Version 2 APIs.
//Read the Symbol IDs in a Component Instance.
int SealReadSymbol(SealSymId ** sids, ComponentInstance * comp)
{
    int i;
    int nsymbol;

    //Seal version check.
    if (V2 != comp->version)
	{
	    INFO("#Seal Symbol requires Version 2.\n");
	    return SEAL_ERR_WRONG_VERSION;
	}

    //Allocate an array for Symbol IDs.
    if (STRING == comp->type)
	{
	    //STRING can have only one Symbol regardless of its size.
	    *sids = (SealSymId *) Malloc(sizeof(SealSymId));
	    nsymbol = 1;
	}
    else
	{
	    *sids = (SealSymId *) Malloc(sizeof(SealSymId) * comp->size);
	    nsymbol = comp->size;
	}

    //Decode the Symbols.
    for (i = 0; i < nsymbol; i++)
	{
	    (*sids)[i] = ntohl(comp->ci.symid[i]);
	}

    return nsymbol;
}

//Set Symbols of Component Instace ${ci} to ${symid}.
int SealSetSymbol(ComponentInstance * comp, const SealSymId * symid)
{
    //Seal version check.
    if (V2 != comp->version)
	{
	    INFO("#Seal Symbol requires Version 2.\n");
	    return SEAL_ERR_WRONG_VERSION;
	}

    return SfAnnotate(&comp->ci, symid);
}

//Annote the ${index}-th element int Component ${compname} by Symbol "symid".
int SealAnnotate(ComponentInstance * comp, int index, const SealSymId symid)
{
    SealSymId *dstsymid = &(comp->ci.symid[index]);
    //Seal version check.
    if (V2 != comp->version)
	{
	    INFO("#Seal Symbol requires Version 2.\n");
	    return SEAL_ERR_WRONG_VERSION;
	}
    *dstsymid = htonl(symid);
    return SEAL_SUCCESS;
}

//Encode a Symbol name into its ID.
SealSymId SealEncodeSym(EncDict * dict, const char *symname)
{
    return FindSymIdByName(dict, symname);
}

//Decode a Symbol ID into its name.
const char *SealDecodeSym(EncDict * dict, SealSymId symid)
{
    return FindSymNameById(dict, symid);
}
