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

#define INTERACTIVE (0)
#define PRINT_FRAME_IN_EMULATION (1)

Seal *seal;
const char *TESTCORE = "testcore.scs";

using namespace std;

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

void PrintComponent(SealFrameCompInst * idx)
{
    int i;

    printf("|%-12s|%-8s|%02d| = [%012lX] ",
	   idx->name, SccGetTypeStr(idx->type), idx->len, (long)idx->val);
    for (i = 0; i < (SccSizeOf(idx->type) * idx->len); i++)
	{
	    printf("%02X ", ((unsigned char *)idx->val)[i]);
	}

    printf("(");
    if (BOOL == idx->type)
	{
	    for (i = 0; i < idx->len; i++)
		{
		    printf("%c", (((unsigned char *)idx->val)[i] ? 'T' : 'F'));
		}
	}
    else if (OCTET == idx->type)
	{
	    printf("OCT vals");
	}
    //Translated info.
    else if (STRING == idx->type)
	{
	    printf("%s", (const char *)idx->val);
	}
    else
	{
	    for (i = 0; i < idx->len; i++)
		{
		    switch (idx->type)
			{
			case INT16:
			    printf("%d", ntohs(((const int16_t *)idx->val)[i]));
			    break;

			case UINT16:
			    printf("%d",
				   ntohs(((const uint16_t *)idx->val)[i]));
			    break;

			case INT32:
			    printf("%d", ntohl(((const int32_t *)idx->val)[i]));
			    break;

			case UINT32:
			    printf("%d",
				   ntohl(((const uint32_t *)idx->val)[i]));
			    break;

			default:
			    break;
			}

		    if (idx->len - 1 > i)
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
        printf("%d: %lu\n", cycle, seal->trace->offset);


#if PRINT_FRAME_IN_EMULATION
	    //Execution display.    
	    printf("===== CYCLE: %02d =====\n", cycle);
	    printf("Frame ID: %ld\n", seal->trace->offset);
	    PrintMapping();
	    PrintFrame(seal->frame);
	    printf("=====************=====\n\n");
#endif
	}

    return;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
	{
	    printf("Usage: %s ${TRACE_FILE_PATH}\n", argv[0]);
	    exit(0);
	}

    //Init a SEAL object.
    seal = InitSeal(TESTCORE, argv[1], SEAL_TRACE_MODE_FIFO);

    //Run Emulation and generate Trace.
    Run();

    //Release the SEAL object.
    FreeSeal(seal);

    return 0;
}
