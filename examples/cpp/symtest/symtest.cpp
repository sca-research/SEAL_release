#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "seal/seal.h"
#include "seal/sealsym.h"

//Event Reference Object.
struct TestRef {
    int i;
    char s[10];
};

//Callback function converting a TestRef into a string.
char *TestRef2String(const void *input)
{
    char strbuf[255] = { 0 };
    char *str = NULL;
    struct TestRef *testref = (struct TestRef *)input;

    snprintf(strbuf, sizeof(strbuf), "obj={%d,%s}", testref->i, testref->s);
    str = (char *)malloc(strnlen(strbuf, sizeof(strbuf)) + 1);
    strcpy(str, strbuf);

    return str;
}

int main(int argc, char *argv[])
{
    Seal *seal;
    Dict *dict;
    Symbol *a, *b, *c;
    SealCoreComponent **comps;
    EventLog *eventlog;
    EventRef er;
    ChainNode *node;
    LeakageEvent *ev;
    struct TestRef refobj = {.i = 1,.s = "hello" };

    //Load in a Core Specification.
    seal =
	InitSeal("testcore.scs", "/tmp/nulltrace", SEAL_TRACE_MODE_CREATE);
    comps = seal->core->components;

    //Set up the Dictionary and its Symbols.
    dict = NewDict();
    a = NewSymbol("a");
    b = NewSymbol("b");
    c = NewSymbol("c");
    AddSymbol(dict, a);
    AddSymbol(dict, b);
    AddSymbol(dict, c);

    //Set up an Event Log.
    eventlog = NewEventLog();

    //Log event with Integer Reference.
    er.i = 1551;
    LogEvent(eventlog, comps[0], a, SEAL_SYM_EVENT_REF_INT, er);
    //Log event with String Reference.
    er.s = "Event 2";
    LogEvent(eventlog, comps[1], b, SEAL_SYM_EVENT_REF_STR, er);
    //Log event with Object Reference.
    er.o = &refobj;
    LogEvent(eventlog, comps[2], c, SEAL_SYM_EVENT_REF_OBJ, er);

    //Configure ToString function for Object Event Reference.
    SealToStr.EventRefObj = TestRef2String;

    //Enumerate over the log and print.
    GetEnum(&node, eventlog->events);
    for (GetEnum(&node, eventlog->events); NULL != node; EnumChain(&node))
	{
	    char *evestr;
	    ev = (LeakageEvent *) node->content;
	    printf("[%s]\n", evestr = SealToStr.Event(ev));
	    free(evestr);
	}

    //Clean.
    DelDict(dict);
    DelEventLog(eventlog);
    FreeSeal(seal);

    return 0;
}
