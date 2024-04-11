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

const char *TESTJSON = "testcore.scs";
const char *REG_TEST_TRACE = "testtrace.stc";
const char *ITA_TEST_TRACE = "/tmp/sealtraceif";

using namespace std;

void ReadRecord(SealCore * core)
{
    SealFrame *frame = NULL;
    SealFrameCompInst rega, regb, exeins, mode, cyclecount;
    SealTrace *trace = NULL;
    int cycle, ret;

    //Init
#if INTERACTIVE
    trace =
	StOpen(REG_TEST_TRACE, core, SEAL_TRACE_MODE_READ,
	       SEAL_TRACE_FLAG_INTERACTIVE);
#else
    trace = StOpen(REG_TEST_TRACE, core, SEAL_TRACE_MODE_READ);
#endif
    frame = SfNewFrame(core);

    //Set up index to components.
    SfFetchFrameCI(frame, &rega, "RegA");
    SfFetchFrameCI(frame, &regb, "RegB");
    SfFetchFrameCI(frame, &exeins, "ExeIns");
    SfFetchFrameCI(frame, &mode, "Mode");
    SfFetchFrameCI(frame, &cyclecount, "CycleCount");

    for (cycle = 0; 0 < (ret = StNextFrame(trace, frame)) && cycle <= 11;
	 cycle++)
	{
	    //Print each Core Component.
	    printf("===== CYCLE: %02d =====\n", cycle);
	    printf("Frame ID: %ld (Header = %d)\n", trace->offset,
		   ntohl(*frame->header));
	    PrintComponent(&rega, core);
	    PrintComponent(&regb, core);
	    PrintComponent(&exeins, core);
	    PrintComponent(&mode, core);
	    PrintComponent(&cyclecount, core);
	    PrintFrame(frame, core);
	    printf("=====************=====\n\n");
	}

    StClose(trace);
    SfDeleteFrame(frame);

    return;
}

int main(int argc, char *argv[])
{
    SealCore *core = NULL;

    if (argc < 2)
	{
	    printf("Usage: readtrace ${TRACE_FILE_PATH}\n");
	    exit(0);
	}
    REG_TEST_TRACE = argv[1];

    //Read a Core.
    core = ScLoadCore(TESTJSON);

    //Read Trace file.
    ReadRecord(core);

    ScDeleteCore(core);
    return 0;
}
