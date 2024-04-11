#ifndef _SEAL_H
#define _SEAL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"

#include "sealconfig.h"
#include "core.h"
#include "trace.h"
#include "frame.h"
#include "chain.h"
#include "symbolic.h"

#define SEAL_SUCCESS (0)
#define SEAL_ERROR (-1)
#define SEAL_EXIST (-2)

//Error code.
#define SEAL_ERR_GENERAL (-1)
#define SEAL_ERR_NO_SUCH_COMPONENT (-2)
#define SEAL_ERR_NO_SUCH_SYMBOL (-3)
#define SEAL_ERR_CORRUPTED_FRAME (-4)
#define SEAL_ERR_WRONG_VERSION (-5)

//Disable interactive trace.
//#define SEAL_TRACE_FLAG_INTERACTIVE (0x1)
    //Acronyms for exporting internal data structures.
    typedef SealCoreComponent SealComponent;
    typedef SealCoreComponentType SealComponentType;

    //APIs for SEAL info.
    extern bool sealinfo;
    void SealSetVerbose(bool newsealinfo);
    void INFO(const char *errmsg);
    int SealErrno();

    //Mapping table for binding Component intances.
    typedef struct __ValMap__ {
	SealFrameCompInst *ci;
	const void *emuvar;
	struct __ValMap__ *next;
    } ValMap;

    //Seal session.
    typedef struct {
	SealCore *core;		//Seal Core specification.
	SealTrace *trace;	//Seal Trace handle.
	SealFrame *frame;	//Seal Frame buffer.
	ValMap *maps;		//Mappings from Seal Trace Frame Indexes to emulator variables.
	Dict *dict;
	EventLog *eventlog;
    } Seal;
    //Initialise a new SEAL object.
    Seal *InitSeal(const char *corepath, const char *tracepath, int tracemode);
    //Close a SEAL object.
    void FreeSeal(Seal * seal);

    //Retrive a hanlde of Component by its name, or NULL if no such Component found.
    SealComponent *SealGetComponent(Seal * seal, const char *compname);

    //User data structure for Component Instance access.
    typedef struct _ComponentInstance_ {
	uint32_t version;
	const char *name;	//Name of the Component.
	SealCoreComponentType type;	//Type of the Component.
	size_t typesize;	//Size of the type of the Component in byte.
	const char *typestr;	//Type of the Component in string
	size_t size;		//Size of the Component in its type.
	CompVal val;		//Pointer to its value.
	//Associated objects.
	SealFrame *frame;	//Pointer to its associated Frame.
	SealFrameCompInst ci;	//Pointer to its low level representation.
    } ComponentInstance;

    //Read the next frame in the trace into  the buffer.
    int SealNextFrame(Seal * seal);
    //Create a new empty Frame.
    SealFrame *SealNewFrame(Seal * seal);
    //Copy a Frame.
    int SealCopyFrame(SealFrame * dstframe, const SealFrame * srcframe);
    //Free a Frame.
    void SealFreeFrame(SealFrame *);
    //Fetch the ${frameno}-th Frame into ${frame}. This does not affect the Trace offset.
    int SealFetchFrame(SealFrame * frame, Seal * seal, unsigned long frameno);
    //Fetch the Component from $frame$ by its name ${compname}.
    int SealFetchComp(ComponentInstance * comp, SealFrame * frame,
		      const char *compname);

    //List at ${complist} all Component Instances within a given Frame. Returns the number of Components.
    int SealListComp(ComponentInstance ** complist, SealFrame * frame);
    //Write a Seal Trace Frame.
    int SealWrite(Seal * seal);
    //Write a Frame ${frame} into ${trace}.
    int SealWriteFrame(Seal * seal, SealFrame * frame);

    //Version 2 APIs.
    //Read the Symbol IDs in a Component Instance. Version 2 only.
    int SealReadSymbol(SealSymId ** sids, ComponentInstance * comp);
    //Set Symbols of Component Instace ${ci} to ${symid}.
    int SealSetSymbol(ComponentInstance * comp, const SealSymId * symid);
    //Annote the ${index}-th element int Component ${compname} by Symbol "symid".
    int SealAnnotate(ComponentInstance * comp, int index,
		     const SealSymId symid);
    //Encode a Symbol name into its ID.
    SealSymId SealEncodeSym(EncDict * dict, const char *symname);
    //Decode a Symbol ID into its name.
    const char *SealDecodeSym(EncDict * dict, SealSymId symid);
#ifdef __cplusplus
}
#endif
#endif
