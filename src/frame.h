#ifndef _FRAME_H
#define _FRAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core.h"
#include "symbolic.h"

    //Internal definition for Seal Component Instances.
    typedef struct _SealTraceCompInst_ {
	uint32_t version;
	//For V1.
	char *name;
	SealCoreComponentType type;
	int len;
	void *val;
	//For V2.
	SealSymId *symid;
    } SealFrameCompInst;

    //Seal Trace Frame.
    typedef struct _SealFrame_ {
	const SealCore *core;
	uint32_t *header;
	uint8_t *body;
	int len;
	unsigned char *buf;
    } SealFrame;

    //Return the size of Frame in bytes given the Core.
    size_t FrameSize(const SealCore * core);

    //Constructor/Destructor of a Seal Frame.
    SealFrame *SfNewFrame(const SealCore * core);
    void SfDeleteFrame(SealFrame * stf);

    //Get an index inside the Frame ${frame} corresponding to the Core Component ${name}.
    int SfFetchFrameCI(const SealFrame * frame,
		       SealFrameCompInst * idx, const char *name);

    //Assign ${ci.val} to ${val}.
    int SfAssign(SealFrameCompInst * ci, const void *val);

    //Annotate ${ci} by ${symid}.
    int SfAnnotate(SealFrameCompInst * ci, const SealSymId * symid);

    //Get target Symbol ID ${targetsymid} in a Frame.
    SealSymId SfGetFrameSymid(SealSymId * targetsymid);

    //Set target Symbol ID ${targetsymid} in a Frame to value specified by ${symid}.
    void SfSetFrameSymid(SealSymId * targetsymid, SealSymId symid);
#ifdef __cplusplus
}
#endif
#endif
