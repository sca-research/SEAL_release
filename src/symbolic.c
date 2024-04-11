#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "seal.h"
#include "sealutl.h"
#include "symbolic.h"

//Initialise a new Code Entry.
CodeEntry *NewCodeEntry(SealSymId symid, const char *symname)
{
    CodeEntry *ce = NULL;

    ce = (CodeEntry *) Malloc(sizeof(CodeEntry));
    ce->symbolid = symid;
    ce->symbolname = CloneString(symname);

    return ce;
}

//Wrapper of DelCodeEntry for chain API.
static void DelCodeEntryC(void *obj)
{
    CodeEntry *ec = (CodeEntry *) obj;

    Free(ec->symbolname);
    Free(ec);
    return;
}

//Free a Code Entry.
void DelCodeEntry(CodeEntry * ce)
{
    DelCodeEntryC(ce);
    return;
}

//Initialise an new Encoding Dictionary.
EncDict *NewEncDict()
{
    EncDict *edict = NULL;
    CodeEntry *nullentry = NULL;

    //Initialise an empty dictionary.
    edict = (EncDict *) Malloc(sizeof(EncDict));
    edict->entry = NULL;
    edict->lasthint = NULL;
    edict->nentry = 0;

    //Add NULL entry.
    nullentry = (CodeEntry *) Malloc(sizeof(CodeEntry));
    nullentry->symbolid = SEAL_SYM_NULL_ID;
    nullentry->symbolname = CloneString(SEAL_SYM_NULL_STR);
    AddCodeEntry(edict, nullentry);

    return edict;
}

//Free an Encoding Dictionary. This also deletes all Symbols within.
void DelEncDict(EncDict * edict)
{
    DelChain(&edict->entry, DelCodeEntryC);
    Free(edict);
    return;
}

//Add ${ce} into ${edict}.
int AddCodeEntry(EncDict * edict, CodeEntry * ce)
{
    edict->lasthint = AddNode(&edict->entry, edict->lasthint, ce);
    edict->nentry++;

    return SEAL_SUCCESS;
}

//Check if two Symbol names are the same. Used by FindSymIdByName().
static int SameSymName(const void *x, const void *y)
{
    const CodeEntry *ce = (const CodeEntry *)x;
    const char *targetname = (const char *)y;

    if (0 == strncmp(ce->symbolname, targetname, SEAL_SYM_LEN))
	{
	    return 1;
	}
    else
	{
	    return 0;
	}

    return 0;
}

//Find Symbol ID by its name.
SealSymId FindSymIdByName(const EncDict * edict, const char *symname)
{
    ChainNode *cenode = NULL;
    CodeEntry *ce = NULL;

    //Find the node with the same Symbol name.
    cenode = SearchNode(edict->entry, SameSymName, symname);
    if (NULL == cenode)
	{			//sym is not in dict.
	    return SEAL_SYM_NULL_ID;
	}

    //Node found. Extract symid.
    ce = (CodeEntry *) cenode->content;

    return ce->symbolid;
}

//Check if two Symbol ID are the same. Used by FindSymNameById.
static int SameSymId(const void *x, const void *y)
{
    const CodeEntry *ce1 = (const CodeEntry *)x;
    const SealSymId *targetid = (const SealSymId *)y;

    return (ce1->symbolid == *targetid ? 1 : 0);
}

//Find Symbol name by its ID.
const char *FindSymNameById(const EncDict * edict, SealSymId symid)
{
    ChainNode *cenode = NULL;
    CodeEntry *ce = NULL;

    //Find the node with the same Symbol name.
    cenode = SearchNode(edict->entry, SameSymId, &symid);
    if (NULL == cenode)
	{			//sym is not in dict.
	    return SEAL_SYM_NULL_STR;
	}

    //Node found. Extract symname.
    ce = (CodeEntry *) cenode->content;

    return ce->symbolname;
}

//Convert the Encoding Dictionary into an array of CodeEntry.
int EncDict2Array(CodeEntry ** cearray, const EncDict * edict)
{
    int i = 0;
    CodeEntry *ce = NULL;
    ChainNode *c = edict->entry;

    *cearray = (CodeEntry *) Malloc(sizeof(CodeEntry) * edict->nentry);

    //Skip the first NULL symbol.
    c = c->next;

    //Copy code entries from the edict to the output array.
    for (i = 0; NULL != c; i++, c = c->next)
	{
	    ce = (CodeEntry *) c->content;
	    (*cearray)[i].symbolid = ce->symbolid;
	    (*cearray)[i].symbolname = ce->symbolname;
	}

    //Ternimate the array by NULL symbol.
    (*cearray)[i].symbolid = SEAL_SYM_NULL_ID;
    (*cearray)[i].symbolname = SEAL_SYM_NULL_STR;

    return (int)edict->nentry;
}

//Import an Encoding Dictionary from a file.
EncDict *ImportEncDict(const char *dictfilepath)
{
    int ret, nlineread = 0;
    EncDict *newdict = NULL;
    FILE *dictfile = NULL;
    size_t linelen = 0;
    char *line = NULL;
    SealSymId symid = 0;
    char *symname = NULL;
    CodeEntry *ce = NULL;
    char infobuf[SEAL_SYM_LEN] = { 0 };

    newdict = NewEncDict();

    //Open dictionary file.
    if (NULL == (dictfile = fopen(dictfilepath, "r")))
	{
	    INFO("#Could not open dictionary file.\n");
	    perror("#fopen(): ");
	    return NULL;
	}

    //Read each line in the file.
    while (true)
	{
	    //Let getline() allocate the space.
	    line = NULL;
	    linelen = 0;
	    ret = getline(&line, &linelen, dictfile);

	    if (0 < ret)	//Successfully fetched a line.
		{
		    //First line header.
		    if (0 == nlineread)
			{
			    //Reserved.
			    nlineread++;
			}
		    else
			{
			    switch (line[0])
				{
				    //Skip blank lines and comment lines begin with '#'.
				case '\n':
				case '#':
				    break;

				    //A normal line.
				default:
				    //Scan Code Entry format: 
				    //  [SYMID]::[SYMNAME]\NewLine
				    ret = sscanf(line, "%08x::%ms\n", &symid,
						 &symname);

				    if (2 > ret)
					{
					    snprintf(infobuf, sizeof(infobuf),
						     "#Corrupted file at L%d\n",
						     nlineread);
					    INFO(infobuf);
					    free(symname);
					    goto ERROR;
					}

				    ce = NewCodeEntry(symid, symname);
				    AddCodeEntry(newdict, ce);
				    free(symname);
				    break;
				}
			    nlineread++;
			}
		    free(line);
		}
	    else if (0 == errno)	//EOF
		{
		    INFO("#ImportEncDict(): EOF reached.");
		    free(line);
		    break;
		}
	    else		//Errors
		{
		    if (-1 == ret)
			{
			    perror("getline():");
			    goto ERROR;
			}
		}

	}

    //Clean.
    fclose(dictfile);

    return newdict;

 ERROR:
    DelEncDict(newdict);
    fclose(dictfile);
    free(line);
    return NULL;
}

//Construct Encoding Dictionary header.
//Reserved API
static inline int SetEDHeader(char *buf, int buflen, EncDict * dict)
{
    memset(buf, 0, buflen);
    return snprintf(buf, buflen - 1, "[]\n");
}

//Encode a CodeEntry into a string.
static inline int CodeEntry2Str(char *buf, int buflen, const CodeEntry * ce)
{
    //Clear the buffer.
    memset(buf, 0, buflen);
    //Encode CodeEntry.
    return snprintf(buf, buflen - 1, "%08X::%s\n", ce->symbolid,
		    ce->symbolname);
}

//Export an Encoding Dictionary into a file.
int ExportEncDict(EncDict * dict, const char *dictfilepath)
{
    FILE *dictfile = NULL;	//Output file.
    char linebuf[SEAL_SYM_LEN] = { 0 };
    int linelen = 0;
    int nentry = 0;
    CodeEntry *ce = NULL;
    int i = 0;

    //Open dictionary file.
    if (NULL == (dictfile = fopen(dictfilepath, "w")))
	{
	    INFO("#Could not create dictionary file.\n");
	    perror("#fopen(): ");
	    goto ERROR;
	}

    //Write Encoding Dictionary header.
    linelen = SetEDHeader(linebuf, sizeof(linebuf), dict);
    fwrite(linebuf, linelen, 1, dictfile);	//TODO: Error ignored.

    //Write Encoding entries.
    nentry = EncDict2Array(&ce, dict);
    for (i = 0; i < nentry - 1; i++)	//Ignore the last entry NULL.
	{
	    linelen = CodeEntry2Str(linebuf, sizeof(linebuf), &ce[i]);
	    fwrite(linebuf, linelen, 1, dictfile);	//TODO: Error ignored.
	}

    //Clean.
    Free(ce);
    fclose(dictfile);

    return SEAL_SUCCESS;

 ERROR:
    Free(ce);
    fclose(dictfile);
    return SEAL_ERROR;
}

//*************************************************************
//***********************IMPRTANT NOTICE***********************
//*************************************************************
//*********************DEVELOPMENT FROZEN**********************
//*************************************************************
//Create a new Symbol from a string.
Symbol *NewSymbol(const char *idstr)
{
    Symbol *sym = NULL;
    sym = (Symbol *) Malloc(sizeof(Symbol));
    sym->idlen = strnlen(idstr, SEAL_SYM_LEN) + 1;
    sym->id = CloneString(idstr);

    return sym;
}

//Free a Symbol.
void DelSymbol(Symbol * sym)
{
    Free((char *)sym->id);
    Free(sym);
    return;
}

//Compare if s1 and s2 are the same by their ID.
bool SameSymbol(Symbol * s1, Symbol * s2)
{
    if (0 == strncmp(s1->id, s2->id, SEAL_SYM_LEN))
	{
	    return true;
	}
    else
	{
	    return false;
	}
}

//Initialise a new Dictionary.
Dict *NewDict()
{
    Dict *dict = NULL;

    //Initialise an empty dictionary.
    dict = (Dict *) Malloc(sizeof(Dict));
    dict->entry = NULL;
    dict->lasthint = NULL;
    dict->mode = SEAL_SYM_DICT_STATIC;
    dict->nentry = 0;
    return dict;
}

//Wrapper of DelSymbol for chain API.
static void DelSymbolC(void *sym)
{
    DelSymbol((Symbol *) sym);
    return;
}

//Free a Dictionary. This also deletes all Symbols within.
void DelDict(Dict * dict)
{
    DelChain(&dict->entry, DelSymbolC);
    Free(dict);
    return;
}

//Wrapper of SameSymbol for chain API.
static int SameSymbolC(const void *s1, const void *s2)
{
    return SameSymbol((Symbol *) s1, (Symbol *) s2);
}

//Find sym in dict. If sym is not in dict, NULL is returned.
Symbol *FindSymbol(const Dict * dict, const Symbol * sym)
{
    ChainNode *symnode = NULL;

    //Find the node with the same Symbol name.
    symnode = SearchNode(dict->entry, SameSymbolC, sym);
    if (NULL == symnode)
	{			//symname is not in dict.
	    return NULL;
	}

    return (Symbol *) symnode->content;
}

//Return 1 if Symbol x has ID ${y}, 0 otherwise.
static int SameSymbolId(const void *x, const void *y)
{
    const Symbol *sym = (Symbol *) x;
    const char *id = (const char *)y;

    if (0 == strncmp(sym->id, id, SEAL_SYM_LEN))
	{
	    return 1;
	}
    else
	{
	    return 0;
	}
}

//Find a Symbol in ${dict} indicated by its ID ${id}. If ${id} 
Symbol *FindSymbolId(const Dict * dict, const char *id)
{
    ChainNode *node = NULL;
    Symbol *sym = NULL;

    //Search in dict if there is a Symbol with ID being ${id}.
    node = SearchNode(dict->entry, SameSymbolId, id);
    if (NULL != node)
	{
	    sym = (Symbol *) node->content;
	}

    return sym;
}

//Add sym into dict.
int AddSymbol(Dict * dict, Symbol * sym)
{
    if (NULL != FindSymbol(dict, sym))
	{			//sym already in dict.
	    return SEAL_SYM_EXIST;
	}

    dict->lasthint = AddNode(&dict->entry, dict->lasthint, sym);
    dict->nentry++;
    return SEAL_SUCCESS;
}

//Initialise a Symbolic Leakage Event.
LeakageEvent *NewLeakageEvent()
{
    LeakageEvent *event = NULL;

    event = (LeakageEvent *) Malloc(sizeof(LeakageEvent));
    event->comp = NULL;
    event->reftype = SEAL_SYM_EVENT_REF_NON;
    memset((void *)&event->ref, 0, sizeof(EventRef));
    event->freeref = NULL;
    event->tostr = NULL;

    return event;
}

//Free a Symbolic Leakage Event.
void DelLeakageEvent(LeakageEvent * eve)
{
    switch (eve->reftype)
	{
	    //Integer ref will be freed along the Event. No action needed.
	case SEAL_SYM_EVENT_REF_INT:
	    break;

	    //Free the reference string.
	case SEAL_SYM_EVENT_REF_STR:
	    Free((char *)eve->ref.s);
	    break;

	    //Memory management of reference objects are done by user. No action needed here.
	case SEAL_SYM_EVENT_REF_OBJ:
	    break;

	    //Refenece Objects with auto free.
	case SEAL_SYM_EVENT_REF_OBJ_AF:
	    if (NULL == eve->freeref)
		{
		    //Use default free function.
		    Free((void *)eve->ref.o);
		}
	    else
		{
		    //Use customised free function.
		    eve->freeref((void *)eve->ref.o);
		}
	    break;

	default:
	    break;
	}
    Free(eve);

    return;
}

//Set a Leakage Event by the given arguments.
//${ref} is ignored SEAL_SYM_EVENT_REF_NON is specified for reftype.
void SetEvent(LeakageEvent * ev, const SealCoreComponent * comp,
	      const Symbol * sym, const EventRefType reftype,
	      const EventRef ref, FreeFunc freeref, SealO2S tostr)
{
    //Copy Component, Symbol and Reference Type.
    ev->comp = comp;
    ev->sym = sym;
    ev->reftype = reftype;

    //Set Event Referecen.
    switch (ev->reftype)
	{
	case SEAL_SYM_EVENT_REF_NON:	//No reference.
	    break;

	case SEAL_SYM_EVENT_REF_INT:	//Integer reference.
	    ev->ref.i = ref.i;
	    break;

	case SEAL_SYM_EVENT_REF_STR:	//String reference.
	    ev->ref.s = CloneString(ref.s);
	    break;

	case SEAL_SYM_EVENT_REF_OBJ:	//Object reference (with and without auto free).
	case SEAL_SYM_EVENT_REF_OBJ_AF:
	    ev->ref.o = ref.o;
	    break;

	default:
	    break;
	}

    //Set Reference free function.
    ev->freeref = freeref;

    //Set Reference to string function.
    ev->tostr = tostr;

    return;
}

//Initialise a new Event Log.
EventLog *NewEventLog()
{
    EventLog *eventlog;

    eventlog = (EventLog *) Malloc(sizeof(EventLog));
    eventlog->fd = 0;
    eventlog->events = NULL;
    eventlog->lasthint = NULL;
    eventlog->nevents = 0;

    return eventlog;
}

//Wrapper for DelLeakageEvent() for chain.
void DelLeakageEventC(void *e)
{
    DelLeakageEvent((LeakageEvent *) e);
    return;
}

//Free an Event Log.
void DelEventLog(EventLog * eventlog)
{
    DelChain(&eventlog->events, DelLeakageEventC);
    Free(eventlog);
    return;
}

//Log into ${el} an Symbolic Leakage Event of ${comp} leaking ${sym} with reference ${ref} of type ${reftype}.
//If ${reftype} is SEAL_SYM_EVENT_REF_NON then ${ref} is ignored.
void LogEvent(EventLog * el, const SealCoreComponent * comp,
	      const Symbol * sym, const EventRefType reftype,
	      const EventRef ref, FreeFunc freeref, SealO2S tostr)
{
    LeakageEvent *ev = NULL;
    ChainNode *newnode = NULL;

    //Generate an Leakage Event by parameters.
    ev = NewLeakageEvent();
    SetEvent(ev, comp, sym, reftype, ref, freeref, tostr);

    //Add ev as a new node in el.
    newnode = AddNode(&el->events, el->lasthint, (void *)ev);
    el->nevents++;

    //Update el for quick adding next time.
    el->lasthint = newnode;

    return;
}
