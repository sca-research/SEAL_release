#ifndef _SYMBOLIC_H
#define _SYMBOLIC_H
#ifdef __cplusplus
extern "C" {
#endif

#include "chain.h"
#include "sealutl.h"
#include "core.h"

#define SEAL_SYM_LEN ((255))
#define SEAL_SYM_EXIST ((1))
#define SEAL_SYM_NOT_FOUND ((2))

#define SEAL_SYM_DICT_STATIC ((0))
#define SEAL_SYM_DICT_DYNAMIC ((1))

#define SEAL_SYM_NULL_ID ((0))
#define SEAL_SYM_NULL_STR (("NULL"))

    //Type of Symbol indexes in Dictionary.
    typedef uint32_t SealSymId;

    //Codebook record.
    typedef struct {
	SealSymId symbolid;
	const char *symbolname;
    } CodeEntry;
    CodeEntry *NewCodeEntry(SealSymId symid, const char *symname);
    void DelCodeEntry(CodeEntry * ce);

    //Codebook.
    typedef struct {
	ChainNode *entry;
	ChainNode *lasthint;
	unsigned int nentry;
    } EncDict;
    //Initialise a new Dictionary.
    EncDict *NewEncDict();
    //Free a Dictionary.
    void DelEncDict(EncDict * edict);
    //Add ${ce} into ${edict}.
    int AddCodeEntry(EncDict * edict, CodeEntry * ce);
    //Find Symbol ID by its name.
    SealSymId FindSymIdByName(const EncDict * edict, const char *symname);
    //Find Symbol name by its ID.
    const char *FindSymNameById(const EncDict * edict, SealSymId symid);
    //Convert the Encoding Dictionary into an array of CodeEntry.
    int EncDict2Array(CodeEntry ** cearray, const EncDict * edict);
    //Import an Encoding Dictionary from a file.
    EncDict *ImportEncDict(const char *dictfilepath);
    //Export an Encoding Dictionary into a file.
    int ExportEncDict(EncDict * dict, const char *dictfilepath);

    //*************************************************************
    //***********************IMPRTANT NOTICE***********************
    //*************************************************************
    //*********************DEVELOPMENT FROZEN**********************
    //*************************************************************
    //Prototype of functions converting Seal objects into strings.
    typedef char *(*SealO2S)(const void *);

    //Seal Symbol.
    typedef struct {
	const char *id;		//Unique identy of the Symbol.
	int idlen;
    } Symbol;
    //Create a new Symbol from a string.
    Symbol *NewSymbol(const char *idstr);
    //Free a Symbol.
    void DelSymbol(Symbol * sym);

    //Seal Dictionary.
    //TODO: Performance and be further optimised using prefix tree instead of chain.
    typedef struct {
	ChainNode *entry;
	ChainNode *lasthint;
	int mode;
	unsigned int nentry;
    } Dict;
    //Initialise a new Dictionary.
    Dict *NewDict();
    //Free a Dictionary.
    void DelDict(Dict * dict);

    //Find ${sym} in ${dict}. If ${sym} is not in ${dict}, NULL is returned.
    Symbol *FindSymbol(const Dict * dict, const Symbol * sym);
    //Find a Symbol in ${dict} indicated by its ID ${id}.
    Symbol *FindSymbolId(const Dict * dict, const char *id);
    //Add sym into dict. No action taken if  ${sym} already exists.
    int AddSymbol(Dict * dict, Symbol * sym);

    //Symbolic Leakage Event Reference.
    typedef enum {
	SEAL_SYM_EVENT_REF_NON,	//Reference not used.
	SEAL_SYM_EVENT_REF_INT,	//Interger reference.
	SEAL_SYM_EVENT_REF_STR,	//String reference.
	SEAL_SYM_EVENT_REF_OBJ,	//Object reference.
	SEAL_SYM_EVENT_REF_OBJ_AF,	//Object reference with auto free.
    } EventRefType;

    typedef union {
	int64_t i;		//Integer reference.
	const char *s;		//String reference.
	const void *o;		//Object reference.
    } EventRef;

    //Symbolic Leakage Events.  
    typedef struct {
	const SealCoreComponent *comp;	//Associated Component.
	const Symbol *sym;	//Associated Symbol.
	EventRefType reftype;	//Type of Reference.
	EventRef ref;		//Reference.
	FreeFunc freeref;	//Reference free function. Only used with Auto Free Reference Objects.
	SealO2S tostr;		//Reference to string function. Only used for Reference Objects.
    } LeakageEvent;
    //Initialise a Symbolic Leakage Event.
    LeakageEvent *NewLeakageEvent();
    //Free a Symbolic Leakage Event.
    void DelLeakageEvent(LeakageEvent * eve);

    //Note a Leakage Event.
    void SetEvent(LeakageEvent * ev, const SealCoreComponent * comp,
		  const Symbol * sym, const EventRefType reftype,
		  const EventRef ref, FreeFunc freeref);

    //Symbolic Leakage Event Log.
    typedef struct {
	int fd;
	ChainNode *events;
	ChainNode *lasthint;
	unsigned int nevents;
    } EventLog;
    //Initialise a new Event Log.
    EventLog *NewEventLog();
    //Free an Event Log.
    void DelEventLog(EventLog * eventlog);

    //Log into ${el} an Symbolic Leakage Event of ${comp} leaking ${sym} with reference ${ref} of type ${reftype}.
    void LogEvent(EventLog * el, const SealCoreComponent * comp,
		  const Symbol * sym, const EventRefType reftype,
		  const EventRef ref, FreeFunc freeref, SealO2S tostr);

#ifdef __cplusplus
}
#endif
#endif
