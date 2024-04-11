#ifndef _SEALSYM_H
#define _SEALSYM_H
#ifdef __cplusplus
extern "C" {
#endif

#include "seal.h"
#include "symbolic.h"

#define SEAL_SYM_MAX_STRLEN (1024)
#define SEAL_SYM_ERR_UNDECLARED (-10)

    typedef Symbol SealSymbol;
    typedef EventRefType SealRefType;
    typedef EventRef SealRef;
    typedef LeakageEvent SealEvent;

    //A Collection of Seal Objects to String APIs.
    struct ToStrIF {
	SealO2S Component;
	SealO2S Symbol;
	SealO2S EventRefObj;
	SealO2S Event;
	SealO2S ComponentInstance;
	SealO2S Frame;
    };
    extern struct ToStrIF SealToStr;

    //String a Seal Component.
    char *Component2s(const void *comp);

    //String a Symbol.
    char *Symbol2s(const void *sym);
    //Callback converting Event Reference Objects to strings. Defined by user.
    extern SealO2S EventRefObj2s;

    //String a Leakage Event.
    char *Event2s(const void *ev);

    //String a Component Instance.
    char *ComponentInstance2s(const void *ev);

    //String a Frame.
    char *Frame2s(const void *ev);

    //Declare a Symbol in Dictionary. Returns a handle to the symbol or NULL if error.
    SealSymbol *Declare(Seal * seal, const char *symid);

    //General method for logging a Leakage Event.
    int LogLeakage(Seal * seal, const SealComponent * comp,
		   SealSymbol * sym, const SealRefType reftype, SealRef ref,
		   FreeFunc freeref, SealO2S tostr);

    //General method for logging a Leakage Event using ID strings of Component and Symbol.
    int LogLeakageStr(Seal * seal, const char *compname,
		      const char *symid, const SealRefType reftype,
		      SealRef ref, FreeFunc freeref, SealO2S tostr);

    //A basic method for logging an Event with Component and Symbol as strings.
    int LogLeakageId(Seal * seal, const char *compname, const char *symid,
		     const SealRefType reftype, SealRef ref);

    //Log a Leakage Event with Component Instance corresponding to ${compname} from ${frame}. 
    //A NULL ${frame} uses the buffer Frame.
    int LogLeakageCI(Seal * seal, const char *compname, const char *symid,
		     SealFrame * frame);

    //Log a Leakage Event with Frame. A NULL ${frame} uses the buffer Frame.
    int LogLeakageFrame(Seal * seal, const char *compname, const char *symid,
			const SealFrame * frame);

    //Generate a list of Symbols in Dictionary as an NULL terminated array.
    SealSymbol **ListDictionary(Seal * seal);

    //Generate a list of Leakage Events in Eventlog as an NULL terminated array.
    SealEvent **ListEventLog(Seal * seal);
#ifdef __cplusplus
}
#endif
#endif
