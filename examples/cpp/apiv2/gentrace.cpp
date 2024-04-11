#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <cjson/cJSON.h>

#include "seal/seal.h"
#include "seal/emulator.h"
#include "seal/sealsym.h"

#define INTERACTIVE (0)
#define PRINT_FRAME_IN_EMULATION (1)

using namespace std;

Seal *seal;
const char *TESTCORE = "testcore.scs";

void PrintFrame(SealFrame * frame)
{
    int i;

    printf("Frame mapping:");
    for (i = 0; i < frame->len; i++)
	{
	    if (!(i % 8))
		printf("\n[%02d]: ", i);
	    printf("%02X ", frame->buf[i]);
	}
    cout << endl;
    return;
}

void PrintComps(SealFrame * frame)
{
    int i;
    unsigned int j;
    int ncomp;
    ComponentInstance *complist;

    ncomp = SealListComp(&complist, frame);
    for (i = 0; i < ncomp; i++)
	{
	    printf("%s:%s:", complist[i].name, complist[i].typestr);

	    if (STRING != complist[i].type)
		{
		    for (j = 0; j < complist[i].size * complist[i].typesize;
			 j++)
			{
			    printf("%02X ", complist[i].val.OCTET[j]);
			}
		    cout << endl;
		}
	    else
		{
		    cout << complist[i].val.STRING << endl;
		}
	}

    free(complist);

    return;
}

void PrintComponent(SealFrameCompInst * ci)
{
    int i;

    printf("|%-12s|%-8s|%02d| = [%012lX] ",
	   ci->name, SccGetTypeStr(ci->type), ci->len, (long)ci->val);
    for (i = 0; i < (SccSizeOf(ci->type) * ci->len); i++)
	{
	    printf("%02X ", ((unsigned char *)ci->val)[i]);
	}

    printf("(");
    if (BOOL == ci->type)
	{
	    for (i = 0; i < ci->len; i++)
		{
		    printf("%c", (((unsigned char *)ci->val)[i] ? 'T' : 'F'));
		}
	}
    else if (OCTET == ci->type)
	{
	    printf("OCT vals");
	}
    //Translated info.
    else if (STRING == ci->type)
	{
	    printf("%s", (const char *)ci->val);
	}
    else
	{
	    for (i = 0; i < ci->len; i++)
		{
		    switch (ci->type)
			{
			case INT16:
			    printf("%d", ntohs(((const int16_t *)ci->val)[i]));
			    break;

			case UINT16:
			    printf("%d",
				   ntohs(((const uint16_t *)ci->val)[i]));
			    break;

			case INT32:
			    printf("%d", ntohl(((const int32_t *)ci->val)[i]));
			    break;

			case UINT32:
			    printf("%d",
				   ntohl(((const uint32_t *)ci->val)[i]));
			    break;

			default:
			    break;
			}

		    if (ci->len - 1 > i)
			printf(",");
		}
	}
    printf(")");
    printf("\n");
    return;
}

void PrintMapping()
{
    ValMap *curr = seal->maps;

    while (NULL != curr)
	{
	    PrintComponent(curr->ci);
	    curr = curr->next;
	}

    return;
}

void Run()
{
    const int MAX_CYCLE = 10;
    uint32_t cycle;
    uint8_t bufflags[3] = { 1, 0, 1 };
    unsigned char bufrega = 0, bufregb[4] = { 0 };
    int16_t bufI16Regs[2] = { 0 };
    uint16_t bufUi16Regs[2] = { 0 };

    const char *bufexeins = "UNDEFINED";
    int32_t bufmode[2] = { 1, 0 };

    SealBind(seal, "Flags", bufflags);
    SealBind(seal, "RegA", &bufrega);
    SealBind(seal, "RegB", bufregb);
    SealBind(seal, "I16Regs", bufI16Regs);
    SealBind(seal, "Ui16Regs", bufUi16Regs);
    SealBind(seal, "ExeIns", bufexeins);
    SealBind(seal, "Mode", bufmode);
    SealBind(seal, "CycleCount", &cycle);

    for (cycle = 0; cycle < MAX_CYCLE; cycle++)
	{
	    //Update Core Components.
	    bufflags[0] ^= 0x1;
	    bufflags[1] ^= 0x1;
	    bufflags[2] ^= 0x1;
	    bufrega++;
	    bufregb[2]++;
	    bufI16Regs[0] -= 1;
	    bufI16Regs[1] -= 2;
	    bufUi16Regs[0] += 1;
	    bufUi16Regs[1] += 2;
	    bufexeins = ((cycle & 1) ? "ODD" : "EVEN");
	    bufmode[1] = cycle % 3;

	    //Rebinding ExeIns.
	    SealBind(seal, "ExeIns", bufexeins);

	    //Sync & Write.
	    SealSync(seal);
	    SealWrite(seal);

#if PRINT_FRAME_IN_EMULATION
	    //Execution display.    
	    printf("===== CYCLE: %02d =====\n", cycle);
	    printf("Frame ID: %ld\n", seal->trace->offset);
	    PrintMapping();
	    PrintFrame(seal->frame);
	    PrintComps(seal->frame);
	    printf("=====************=====\n\n");
#endif
	}

    return;
}

void PrintDict(Seal * seal)
{
    int i;
    char *str;
    SealSymbol **symlist;

    printf("Dictionary @ %p:\n", seal->dict);
    symlist = ListDictionary(seal);
    for (i = 0; NULL != symlist[i]; i++)
	{
	    str = SealToStr.Symbol(symlist[i]);
	    printf("@%p : %s\n", symlist[i], str);
	    free(str);
	}
    free(symlist);

    return;
}

void PrintEventLog(Seal * seal)
{
    int i;
    char *str;
    SealEvent **evelist;

    printf("Event Log @ %p:\n", seal->eventlog);
    evelist = ListEventLog(seal);
    for (i = 0; NULL != evelist[i]; i++)
	{
	    str = SealToStr.Event(evelist[i]);
	    printf("@%p : %s\n", evelist[i], str);
	    free(str);
	}
    free(evelist);

    return;
}

struct TestObj {
    int i;
    const char *s;
};

char *TestObj2s(const void *refobj)
{
    const size_t strlen = 255;
    char *str;
    TestObj *to = (TestObj *) refobj;

    str = (char *)malloc(strlen);
    snprintf(str, strlen, "%04d|%s", to->i, to->s);

    return str;
}

void TestSymbolic(Seal * seal)
{
    SealComponent *evrega, *evregb, *evcyclecount;	//Test Components from Core.
    SealSymbol *syms[3] = { 0 };	//Symbols involved.
    SealRef ref = {.i = 0 };	//Leakage Event Reference.

    TestObj refobj, *refobjaf;

    //Obtain Component handles.
    evrega = SealGetComponent(seal, "RegA");
    evregb = SealGetComponent(seal, "RegB");
    evcyclecount = SealGetComponent(seal, "CycleCount");

    //Dictionary in static mode (default, Symbols needs to be declared before use).
    PrintDict(seal);
    syms[0] = Declare(seal, "Sym1");
    PrintDict(seal);
    syms[1] = Declare(seal, "Sym2");
    PrintDict(seal);
    syms[2] = Declare(seal, "Sym3");
    PrintDict(seal);

    //Print empty Event Log.
    PrintEventLog(seal);
    //Log event: (RegA, Sym1, N/A)
    LogLeakage(seal, evrega, syms[0], SEAL_SYM_EVENT_REF_NON, ref, NULL,
	       NULL);
    PrintEventLog(seal);
    //Log event: (RegB, Sym2, s"hello,world")
    ref.s = "hello,world";
    LogLeakage(seal, evregb, syms[1], SEAL_SYM_EVENT_REF_STR, ref, NULL,
	       NULL);
    PrintEventLog(seal);
    //Log event: (CycleCount, Sym3, i1551)
    ref.i = 1551;
    LogLeakage(seal, evcyclecount, syms[2], SEAL_SYM_EVENT_REF_INT, ref, NULL,
	       NULL);
    PrintEventLog(seal);

    //Log event: (RegB, Sym3, refobj)
    refobj.i = 1;
    refobj.s = "Ref Object";
    ref.o = &refobj;
    LogLeakageId(seal, "RegB", "Sym3", SEAL_SYM_EVENT_REF_OBJ, ref);
    PrintEventLog(seal);

    SealToStr.EventRefObj = TestObj2s;

    //Log event: (RegA, Sym1, refobjaf)
    refobjaf = (struct TestObj *)malloc(sizeof(struct TestObj));
    refobjaf->i = 2;
    refobjaf->s = "Auto Free Object";
    ref.o = refobjaf;
    LogLeakageId(seal, "RegA", "Sym1", SEAL_SYM_EVENT_REF_OBJ_AF, ref);
    PrintEventLog(seal);

    //Change to Dynamic Dictionary.
    seal->dict->mode = SEAL_SYM_DICT_DYNAMIC;

    LogLeakageId(seal, "RegA", "Sym4", SEAL_SYM_EVENT_REF_NON, ref);
    PrintEventLog(seal);

    LogLeakageCI(seal, "RegA", "Sym2", seal->frame);
    PrintEventLog(seal);

    LogLeakageFrame(seal, "RegB", "Sym1", seal->frame);
    PrintEventLog(seal);

    return;
}

int main(int argc, char *argv[])
{

    if (argc < 2)
	{
	    printf("Usage: gentrace ${TRACE_FILE_PATH}\n");
	    exit(0);
	}

    //Init a SEAL object.
    seal = InitSeal(TESTCORE, argv[1], SEAL_TRACE_MODE_CREATE);

    //Run Emulation and generate Trace.
    Run();

    //Symbolic module test.
    //printf("===== Seal Symbolic =====\n");
    //TestSymbolic(seal);

    //Release the SEAL object.
    FreeSeal(seal);

    return 0;
}
