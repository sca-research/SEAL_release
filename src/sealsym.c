#include <stdio.h>
#include <string.h>

#include "sealutl.h"
#include "symbolic.h"
#include "seal.h"

#include "sealsym.h"

//Default ToString functions. 
struct ToStrIF SealToStr = {
    .Component = Component2s,	//Components to string.
    .Symbol = Symbol2s,		//Symbols to string.
    .EventRefObj = NULL,	//Leakage Event Reference Object to string. Defined by user.
    .Event = Event2s,
    //TODO: Provide default method to String the following:
    .ComponentInstance = ComponentInstance2s,
    .Frame = Frame2s,
};

//Return a string of a Seal Component.
char *Component2s(const void *obj)
{
    const SealCoreComponent *comp = (const SealCoreComponent *)obj;
    return CloneString(comp->name);
}

//Return a string of a Symbol.
char *Symbol2s(const void *obj)
{
    const Symbol *sym = (const Symbol *)obj;
    return CloneString(sym->id);
}

char *ComponentInstance2s(const void *refobj)
{
    const size_t strlen = 255;
    char *str;

    ComponentInstance *ci = (ComponentInstance *) refobj;

    str = (char *)malloc(strlen);
    memset(str, 0, strlen);

    snprintf(str, strlen, "%s:%s:%X", ci->name, ci->typestr, ci->val.OCTET[0]);

    return str;
}

char *Frame2s(const void *refobj)
{
    char *str;
    const size_t strlen = 255;
    int i, nci;
    ComponentInstance *cilist;
    SealFrame *frame = (SealFrame *) refobj;

    str = (char *)malloc(strlen);
    memset(str, 0, strlen);

    nci = SealListComp(&cilist, frame);
    for (i = 0; i < nci; i++)
	{
	    char *cistr = SealToStr.ComponentInstance((void *)&cilist[i]);
	    strcat(str, "[");
	    strcat(str, cistr);
	    strcat(str, "] ");
	    free(cistr);
	}
    free(cilist);
    return str;
}

//Interface for user defined Event Reference Object to String function.
SealO2S EventRefObj2s = NULL;

//String a Leakage Event.
char *Event2s(const void *obj)
{
    char evestrbuf[SEAL_SYM_MAX_STRLEN] = { 0 };
    char *compstr, *symstr, *refobjstr = NULL, *evestr;
    int evebuflen_remain = sizeof(evestrbuf), complen, symlen;
    const LeakageEvent *ev = (const LeakageEvent *)obj;

    //Put Component into the buffer.
    compstr = SealToStr.Component(ev->comp);
    complen = strnlen(compstr, SEAL_SYM_MAX_STRLEN);
    strncat(evestrbuf, compstr, evebuflen_remain);
    evebuflen_remain -= complen;
    strncat(evestrbuf, SEAL_SYM_EVENT_STRING_DELIMITER, evebuflen_remain);
    evebuflen_remain--;

    //Put Symbol into the buffer.
    symstr = SealToStr.Symbol(ev->sym);
    symlen = strnlen(symstr, SEAL_SYM_MAX_STRLEN);
    strncat(evestrbuf, symstr, evebuflen_remain);
    evebuflen_remain -= symlen;
    strncat(evestrbuf, SEAL_SYM_EVENT_STRING_DELIMITER, evebuflen_remain);
    evebuflen_remain--;

    switch (ev->reftype)
	{
	    //Concatenate interger-converted-to-string reference.
	case SEAL_SYM_EVENT_REF_INT:
	    snprintf(evestrbuf + strlen(evestrbuf), evebuflen_remain, "%ld",
		     ev->ref.i);
	    break;

	    //Concatenate string reference.
	case SEAL_SYM_EVENT_REF_STR:
	    snprintf(evestrbuf + strlen(evestrbuf), evebuflen_remain, "%s",
		     ev->ref.s);
	    break;

	    //Concatenate Object-converted-to-string reference. The convergence is specified by user.
	case SEAL_SYM_EVENT_REF_OBJ:
	case SEAL_SYM_EVENT_REF_OBJ_AF:
	    if (NULL != ev->tostr)
		{
		    refobjstr = ev->tostr(ev->ref.o);
		    snprintf(evestrbuf + strlen(evestrbuf), evebuflen_remain,
			     "%s", refobjstr);

		}
	    else if (NULL != SealToStr.EventRefObj)
		{
		    refobjstr = SealToStr.EventRefObj(ev->ref.o);
		    snprintf(evestrbuf + strlen(evestrbuf), evebuflen_remain,
			     "%s", refobjstr);
		}
	    else		//Print "N/A" is Object Event Reference to string is not configured.
		{
		    strncat(evestrbuf, "N/A", evebuflen_remain);
		}

	    break;

	    //The reference field should be absent.
	case SEAL_SYM_EVENT_REF_NON:
	    strncat(evestrbuf, "Non", evebuflen_remain);
	    break;
	default:
	    break;
	}

    //Copy the buffer into a returning string.
    evestr = CloneString(evestrbuf);

    Free(refobjstr);
    Free(symstr);
    Free(compstr);

    return evestr;
}

//Declare a Symbol in Dictionary indicated by ${symid}. 
//If the Symbol is already in the dictionary then SEAL_SYM_EXIST is returned.
SealSymbol *Declare(Seal * seal, const char *symid)
{
    int ret;
    SealSymbol *sym = NULL;

    //Generate a new Symbol with ID ${symid}.
    sym = NewSymbol(symid);

    //Add the new symbol into dict.
    ret = AddSymbol(seal->dict, sym);
    if (SEAL_SYM_EXIST == ret)	//Symbol alread declared.
	{
	    //Relocate sym to the existing one.
	    Free(sym);
	    sym = FindSymbolId(seal->dict, symid);
	}

    return sym;
}

//General method for logging a Leakage Event.
int LogLeakage(Seal * seal, const SealComponent * comp,
	       SealSymbol * sym, const SealRefType reftype, SealRef ref,
	       FreeFunc freeref, SealO2S tostr)
{
    Dict *dict = seal->dict;
    EventLog *eventlog = seal->eventlog;

    //Check if the Symbol is declared.
    if (NULL == FindSymbol(seal->dict, sym))
	{
	    //Symbol not declared.
	    INFO("#Symbol not declared.\n");
	    switch (seal->dict->mode)
		{
		    //Static mode requires a Symbol declared before usage.
		case SEAL_SYM_DICT_STATIC:
		    return SEAL_SYM_ERR_UNDECLARED;
		    break;

		    //Dynamic mode adds the Symbol automatically.
		case SEAL_SYM_DICT_DYNAMIC:
		    AddSymbol(dict, sym);
		    break;

		default:
		    return SEAL_ERROR;
		    break;
		}
	}

    //Log the Leakage Event.
    LogEvent(eventlog, comp, sym, reftype, ref, freeref, tostr);

    return SEAL_SUCCESS;
}

//General method for logging a Leakage Event using ID strings of Component and Symbol.
int LogLeakageStr(Seal * seal, const char *compname,
		  const char *symid, const SealRefType reftype, SealRef ref,
		  FreeFunc freeref, SealO2S tostr)
{
    SealComponent *comp;
    SealSymbol *sym;

    //Obtain Component handle.
    if (NULL == (comp = SealGetComponent(seal, compname)))
	{
	    //Component given by compname does not exist in the Core.
	    return SEAL_ERR_NO_SUCH_COMPONENT;
	}

    //Obtain Symbol handle.
    if (NULL == (sym = FindSymbolId(seal->dict, symid)))
	{
	    //Symbol given by symid does not exist in the Dictionary.
	    switch (seal->dict->mode)
		{
		    //Return error if dict is static.
		case SEAL_SYM_DICT_STATIC:
		    return SEAL_ERR_NO_SUCH_SYMBOL;
		    break;
		    //Declare ${symid} if dict is dynamic.
		case SEAL_SYM_DICT_DYNAMIC:
		    sym = Declare(seal, symid);
		    break;
		default:
		    break;
		}
	}

    //Log the event.
    return LogLeakage(seal, comp, sym, reftype, ref, freeref, tostr);
}

//A basic method for logging an Event with Component and Symbol as strings.
int LogLeakageId(Seal * seal, const char *compname,
		 const char *symid, const SealRefType reftype, SealRef ref)
{
    return LogLeakageStr(seal, compname, symid, reftype, ref, NULL, NULL);
}

//Log a Leakage Event with Component Instance corresponding to ${compname} from ${frame}. 
//A NULL ${frame} uses the buffer Frame.
int LogLeakageCI(Seal * seal, const char *compname, const char *symid,
		 SealFrame * frame)
{
    int retcode = 0;
    ComponentInstance *ci = NULL;
    SealRef ref;

    //Use the buffer frame if ${frame} is not specified.
    if (NULL == frame)
	{
	    frame = seal->frame;
	}

    //Fetch Component Instance.
    ci = (ComponentInstance *) Malloc(sizeof(ComponentInstance));
    retcode = SealFetchComp(ci, frame, compname);
    if (SEAL_SUCCESS != retcode)
	{
	    Free(ci);
	    return retcode;
	}

    //Log the Event.
    ref.o = ci;
    retcode =
	LogLeakageStr(seal, compname, symid, SEAL_SYM_EVENT_REF_OBJ_AF, ref,
		      NULL, SealToStr.ComponentInstance);
    if (SEAL_SUCCESS != retcode)
	{
	    Free(ci);
	    return retcode;
	}

    return SEAL_SUCCESS;
}

//Free the Reference Frame.
void FreeRefFrame(void *ro)
{
    SealFrame *frame = (SealFrame *) ro;
    SealFreeFrame(frame);
    return;
}

//Log a Leakage Event with Frame. A NULL Frame uses the buffer Frame.
int LogLeakageFrame(Seal * seal, const char *compname, const char *symid,
		    const SealFrame * frame)
{
    int retcode = 0;
    SealFrame *logframe = NULL;	//Frame stored in the log.
    EventRef ref;

    //Use the buffer frame if ${frame} is not specified.
    if (NULL == frame)
	{
	    frame = seal->frame;
	}

    //Copy ${frame} to ${logframe}
    if (NULL == (logframe = SealNewFrame(seal)))
	{
	    INFO("#Cannot create new frame.\n");
	    return SEAL_ERROR;
	}
    retcode = SealCopyFrame(logframe, frame);
    if (SEAL_SUCCESS != retcode)
	{
	    INFO("#Copy frame failed.\n");
	    goto error;
	}

    //Log the Event with ${logframe}.
    ref.o = logframe;
    retcode =
	LogLeakageStr(seal, compname, symid, SEAL_SYM_EVENT_REF_OBJ_AF, ref,
		      FreeRefFrame, SealToStr.Frame);
    if (SEAL_SUCCESS != retcode)
	{
	    INFO("#LogLeakageStr() failed.\n");
	    goto error;
	}

    return SEAL_SUCCESS;

 error:
    Free(logframe);
    return retcode;
}

//Generate a list of Symbols in Dictionary as an NULL terminated array.
SealSymbol **ListDictionary(Seal * seal)
{
    int i;
    ChainNode *node = NULL;
    SealSymbol **symarray = { NULL };

    symarray =
	(SealSymbol **) malloc(sizeof(SealSymbol *) * (seal->dict->nentry + 1));
    for (i = 0, node = seal->dict->entry; node != NULL; i++, node = node->next)
	{
	    symarray[i] = (SealSymbol *) node->content;
	}
    symarray[i] = NULL;

    //TODO: memory crash bug in following code.
#if 0
    n = ListChain(&symarray, seal->dict->entry, seal->dict->nentry);
    //n = ListChain(&symarray, seal->dict->entry, 0);
    printf("%d\n", seal->dict->nentry);
    printf("%d\n", n);
#endif

    return symarray;
}

//Generate a list of Leakage Events in Eventlog as an NULL terminated array.
SealEvent **ListEventLog(Seal * seal)
{
    int i;
    ChainNode *node = NULL;
    SealEvent **evearray = { NULL };

    evearray =
	(SealEvent **) malloc(sizeof(SealEvent *) *
			      (seal->eventlog->nevents + 1));
    for (i = 0, node = seal->eventlog->events; node != NULL;
	 i++, node = node->next)
	{
	    evearray[i] = (SealEvent *) node->content;
	}
    evearray[i] = NULL;

    return evearray;
}
