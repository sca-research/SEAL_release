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

#include "print.hpp"

#define INTERACTIVE (0)
#define PRINT_FRAME_IN_EMULATION (1)

const char *TESTCORE = "testcore.scs";

using namespace std;

void UpdateSym(SealSymId * regbsym, int len)
{
    int i;

    for (i = 0; i < len; i++)
	{
	    regbsym[i]++;
	}

    return;
}

void RunEmulation(SealTrace * trace, SealFrame * frame)
{
    const SealCore *core = trace->core;
    const int MAX_CYCLE = 10;
    uint32_t cycle;
    uint8_t bufflags[3] = { 1, 0, 1 };
    unsigned char bufrega = 0, bufregb[4] = { 0 };
    int16_t bufI16Regs[2] = { 0 };
    uint16_t bufUi16Regs[2] = { 0 };

    SealSymId regbsym[] = { 0, 1, 2, 3 };

    const char *bufexeins;
    int32_t bufmode[2] = { 1, 0 };
    SealFrameCompInst flags, rega, regb, i16Regs, ui16Regs, exeins, mode,
	cyclecount;

    SfFetchFrameCI(frame, &flags, "Flags");
    SfFetchFrameCI(frame, &rega, "RegA");
    SfFetchFrameCI(frame, &regb, "RegB");
    SfFetchFrameCI(frame, &i16Regs, "I16Regs");
    SfFetchFrameCI(frame, &ui16Regs, "Ui16Regs");
    SfFetchFrameCI(frame, &exeins, "ExeIns");
    SfFetchFrameCI(frame, &mode, "Mode");
    SfFetchFrameCI(frame, &cyclecount, "CycleCount");

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

	    //Update the Frame.
	    SfAssign(&flags, &bufflags);
	    SfAssign(&rega, &bufrega);
	    SfAssign(&regb, &bufregb);
	    SfAssign(&i16Regs, &bufI16Regs);
	    SfAssign(&ui16Regs, &bufUi16Regs);
	    SfAssign(&exeins, bufexeins);
	    SfAssign(&mode, bufmode);
	    SfAssign(&cyclecount, &cycle);

	    //Annotate RegB.
	    SfAnnotate(&regb, regbsym);
	    UpdateSym(regbsym, int (sizeof(regbsym) / sizeof(SealSymId)));

#if PRINT_FRAME_IN_EMULATION
	    printf("===== CYCLE: %02d =====\n", cycle);
	    //Print each Core Component.
	    printf("Frame ID: %ld\n", trace->offset);
	    PrintComponent(&flags, core);
	    PrintComponent(&rega, core);
	    PrintComponent(&regb, core);
	    PrintComponent(&i16Regs, core);
	    PrintComponent(&ui16Regs, core);
	    PrintComponent(&exeins, core);
	    PrintComponent(&mode, core);
	    PrintComponent(&cyclecount, core);
	    PrintFrame(frame, core);
	    printf("=====************=====\n\n");
	    StAddFrame(trace, frame);
#endif
	}

    return;
}

void EmuRecord(SealCore * core, char *tracepath)
{
    SealFrame *frame = NULL;
    SealTrace *trace = NULL;

    frame = SfNewFrame(core);
    trace = StOpen(tracepath, core, SEAL_TRACE_MODE_CREATE);
    RunEmulation(trace, frame);
    StClose(trace);

    SfDeleteFrame(frame);

    return;
}

int main(int argc, char *argv[])
{
    SealCore *core = NULL;

    if (argc < 2)
	{
	    printf("Usage: gentrace ${TRACE_FILE_PATH}\n");
	    exit(0);
	}
    //Read a Core.
    core = ScLoadCore(TESTCORE);

    //Run Emulation and generate Trace.
    EmuRecord(core, argv[1]);

    ScDeleteCore(core);
    return 0;
}
