#include <arpa/inet.h>
#include <string.h>

#include "frame.h"
#include "sealutl.h"
#include "seal.h"

//Calculate the Version 1 Frame length.
size_t FrameLen_V1(const SealCore * core)
{
    int i;
    size_t len;
    SealCoreComponent **comps = core->components;

    //Header
    len = sizeof(uint32_t);

    //Component Instances.
    for (i = 0; i < core->ncomponents; i++)
	{
	    //Size of each variables * number of variables.
	    len += SccSizeOf(comps[i]->type) * comps[i]->len;
	}

    return len;
}

//Calculate the Version 2 Frame length.
size_t FrameLen_V2(const SealCore * core)
{
    int i;
    size_t len;
    SealCoreComponent **comps = core->components;

    //Header
    len = sizeof(uint32_t);

    //Component Instances.
    for (i = 0; i < core->ncomponents; i++)
	{

	    //Size of each variables * number of variables.
	    if (STRING == comps[i]->type)
		{
		    len += (SccSizeOf(comps[i]->type) * comps[i]->len) + 4;
		}
	    else
		{
		    len += (SccSizeOf(comps[i]->type) + 4) * comps[i]->len;
		}

	}

    return len;
}

//Return the size of Frame in bytes given the Core.
size_t FrameSize(const SealCore * core)
{
    size_t framesize = 0;

    switch (core->versionid)
	{
	case V1:
	    framesize = FrameLen_V1(core);
	    break;

	case V2:
	    framesize = FrameLen_V2(core);
	    break;

	default:
	    INFO("#Unknown Core version.\n");
	    framesize = 0;
	    break;
	}

    return framesize;
}

//Constructor and destructor of Seal Trace Frame.
SealFrame *SfNewFrame(const SealCore * core)
{
    SealFrame *frame = NULL;

    //Allocate memory for new Seal Trace.
    frame = (SealFrame *) Malloc(sizeof(SealFrame));

    //Save the corresponding Core.
    frame->core = core;

    //Initialise Frame.
    if (0 == (frame->len = FrameSize(core)))
	{
	    INFO("#Frame allocation failed.\n");
	    SfDeleteFrame(frame);
	    return NULL;
	}

    //Allocate  buffer.
    frame->buf = (unsigned char *)Malloc(frame->len);

    //Init header and body pointer.
    frame->header = (uint32_t *) frame->buf;
    *frame->header = htonl(core->versionid);
    frame->body = frame->buf + sizeof(*frame->header);

    return frame;
}

void SfDeleteFrame(SealFrame * frame)
{
    Free(frame->buf);
    Free(frame);
    return;
}

//Implementation of Version 1 Frame Fetch.
int FetchFrameCI_V1(const SealFrame * frame,
		    SealFrameCompInst * ci, const char *name)
{
    int i;
    bool found = false;
    SealCoreComponent *corecomp = NULL;
    int offset = 0;

    for (i = 0; i < frame->core->ncomponents; i++)
	{
	    corecomp = frame->core->components[i];

	    //Find the corresponding Core Component by matching the names.
	    if (!strncmp(name, corecomp->name, SEAL_CORE_MAXSTRLEN))
		{
		    //Set Frame Index.
		    ci->name = corecomp->name;
		    ci->type = corecomp->type;
		    ci->len = corecomp->len;
		    ci->val = &(frame->body[offset]);
		    found = true;
		    break;
		}
	    else
		{
		    //Update offset by iterated components.
		    offset += SccSizeOf(corecomp->type) * corecomp->len;
		}
	}
    if (!found)
	{
	    INFO("#Index \"");
	    INFO(name);
	    INFO("\" not found.\n");
	    return SEAL_ERROR;
	}

    return SEAL_SUCCESS;
}

//Implementation of Version 1 Frame Fetch.
int FetchFrameCI_V2(const SealFrame * frame,
		    SealFrameCompInst * ci, const char *name)
{
    int i;
    bool found = false;
    SealCoreComponent *corecomp = NULL;
    int offset = 0;

    for (i = 0; i < frame->core->ncomponents; i++)
	{
	    corecomp = frame->core->components[i];

	    //Find the corresponding Core Component by matching the names.
	    if (!strncmp(name, corecomp->name, SEAL_CORE_MAXSTRLEN))
		{
		    //Set Frame Component Instance.
		    ci->name = corecomp->name;
		    ci->type = corecomp->type;
		    ci->len = corecomp->len;
		    ci->val = &(frame->body[offset]);

		    //Set Symbol id. Version 2 only.
		    ci->symid =
			(uint32_t *) ((char *)ci->val +
				      (SccSizeOf(ci->type) * ci->len));

		    found = true;
		    break;
		}
	    else
		{
		    //Update offset by iterated components.
		    if (STRING == corecomp->type)
			{
			    offset +=
				SccSizeOf(corecomp->type) * corecomp->len + 4;
			}
		    else
			{
			    offset +=
				(SccSizeOf(corecomp->type) + 4) * corecomp->len;
			}
		}
	}
    if (!found)
	{
	    INFO("#Index \"");
	    INFO(name);
	    INFO("\" not found.\n");
	    return SEAL_ERROR;
	}

    return SEAL_SUCCESS;
}

//Fetch ${ci} to the Component Instance specified by ${name} in the frame $frame$.
int SfFetchFrameCI(const SealFrame * frame,
		   SealFrameCompInst * ci, const char *name)
{
    int ret;

    ci->version = frame->core->versionid;
    switch (ci->version)
	{
	case V1:
	    ret = FetchFrameCI_V1(frame, ci, name);
	    break;

	case V2:
	    ret = FetchFrameCI_V2(frame, ci, name);
	    break;

	default:
	    INFO("#Version not supported.\n");
	    ci->version = 0;
	    return SEAL_ERROR;
	}

    return ret;
}

//Implementation of Frame version 1 Assign.
int Assign_V1(SealFrameCompInst * ci, const void *val)
{
    int i;
    int len = 0;
    SealDataPointer src, dst;

    //Set length.
    len = ci->len;

    //Set values according to type.
    switch (ci->type)
	{
	case BOOL:
	    src.BOOL = (uint8_t *) val;
	    dst.BOOL = (uint8_t *) ci->val;
	    for (i = 0; i < len; i++)
		{
		    dst.BOOL[i] = (src.BOOL[i] ? 0x01 : 0x00);
		}
	    break;

	case OCTET:
	    src.OCTET = (uint8_t *) val;
	    dst.OCTET = (uint8_t *) ci->val;
	    for (i = 0; i < len; i++)
		{
		    dst.OCTET[i] = src.OCTET[i];
		}
	    break;

	case STRING:
	    src.STRING = (char *)val;
	    dst.STRING = (char *)ci->val;
	    memset(ci->val, 0, ci->len);
	    strncpy(dst.STRING, src.STRING, ci->len - 1);
	    break;

	case INT16:
	    src.INT16 = (int16_t *) val;
	    dst.INT16 = (int16_t *) ci->val;
	    for (i = 0; i < len; i++)
		{
		    dst.INT16[i] = htons(src.INT16[i]);
		}
	    break;

	case UINT16:
	    src.UINT16 = (uint16_t *) val;
	    dst.UINT16 = (uint16_t *) ci->val;
	    for (i = 0; i < len; i++)
		{
		    dst.UINT16[i] = htons(src.UINT16[i]);
		}
	    break;

	case INT32:
	    src.INT32 = (int32_t *) val;
	    dst.INT32 = (int32_t *) ci->val;
	    for (i = 0; i < len; i++)
		{
		    dst.INT32[i] = htonl(src.INT32[i]);
		}
	    break;

	case UINT32:
	    src.UINT32 = (uint32_t *) val;
	    dst.UINT32 = (uint32_t *) ci->val;
	    for (i = 0; i < len; i++)
		{
		    dst.UINT32[i] = htonl(src.UINT32[i]);
		}
	    break;

	case UNDEFINED:
	default:
	    INFO("#Unsupported component type.\n");
	    return SEAL_ERROR;
	    break;
	}
    return SEAL_SUCCESS;
}

//Implementation of Frame version 1 Assign.
int Assign_V2(SealFrameCompInst * ci, const void *val)
{
    //Assign is the same as V1.
    return Assign_V1(ci, val);
}

//Assign ${ci.val} to ${val}.
int SfAssign(SealFrameCompInst * ci, const void *val)
{
    int ret;

    switch (ci->version)
	{
	case V1:
	    ret = Assign_V1(ci, val);
	    break;
	case V2:
	    ret = Assign_V2(ci, val);
	    break;
	default:
	    INFO("#Version not supported.\n");
	    return SEAL_ERROR;
	}

    return ret;
}

//Annotate ${ci} by ${symid}.
int SfAnnotate(SealFrameCompInst * ci, const SealSymId * symid)
{
    int i;
    if (V2 != ci->version)
	{
	    INFO("#Annotate requires V2.\n");
	    return SEAL_ERR_WRONG_VERSION;
	}

    for (i = 0; i < ci->len; i++)
	{
	    ci->symid[i] = htonl(symid[i]);
	}

    return SEAL_SUCCESS;
}

//Get target Symbol ID ${targetsymid} in a Frame.
SealSymId SfGetFrameSymid(SealSymId * targetsymid)
{
    return ntohl(*targetsymid);
}

//Set target Symbol ID ${targetsymid} in a Frame to value specified by ${symid}.
void SfSetFrameSymid(SealSymId * targetsymid, SealSymId symid)
{
    *targetsymid = htonl(symid);
    return;
}
