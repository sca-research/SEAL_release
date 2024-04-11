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

Seal *seal;
const char *TESTCORE = "testcore.scs";

using namespace std;

void PrintOctetComponent(ComponentInstance comp)
{
    int ret;
    unsigned long i;
    SealSymId *sid;
    printf("{Name:%s, ", comp.name);
    printf("Type:%d, ", comp.type);
    printf("Type size:%lu, ", comp.typesize);
    printf("Type string:%s, ", comp.typestr);
    printf("Size:%lu, ", comp.size);

    printf("Value:[");
    for (i = 0; i < comp.size; i++)
	{
	    printf("%02X ", comp.val.OCTET[i]);
	}
    printf("]}");

    //Symbols
    printf("Symbols:[");
    if (2 >= comp.version)
	{
	    ret = SealReadSymbol(&sid, &comp);
	    for (int j = 0; j < ret; j++)
		{
		    printf("%d ", sid[j]);
		}
	}
    printf("]");

    printf("\n");
    return;
}

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

int main(int argc, char *argv[])
{
    int i, ret;
    int frameid = 0;
    SealFrame *frame;
    ComponentInstance comp;

    if (argc < 2)
	{
	    printf("Usage: %s ${TRACE_FILE_PATH}\n", argv[0]);
	    exit(0);
	}

    //Init a SEAL object.
    seal = InitSeal(TESTCORE, argv[1], SEAL_TRACE_MODE_READ);
    frame = SealNewFrame(seal);

    printf("#Sequential reading:\n");
    do
	{
	    printf("Offset=%lu\n", seal->trace->offset);
	    frameid = SealNextFrame(seal);
	    if (0 != frameid)
		{
		    printf("======Frame %d read.======\n", frameid);
		    SealCopyFrame(frame, seal->frame);
		    SealFetchComp(&comp, frame, "RegB");

		    PrintOctetComponent(comp);
		    PrintFrame(frame);
		    printf("\n");
		}
	    else
		{
		    printf("#Trace file ended.\n");
		}
	}
    while (0 != frameid);

    printf("#Indexed reading:\n");
    SealSetVerbose(true);
    for (i = 0; i < 20; i++)
	{
	    ret = SealFetchFrame(frame, seal, i);
	    if (0 > ret)
		{
		    printf("#FetchFrame() error.\n");
		}
	    else if (0 == ret)
		{
		    printf("#Frame %d does not exist.\n", i);
		}
	    else
		{
		    printf("\n");
		    printf("Frame %02d\n", i);
		    SealFetchComp(&comp, frame, "RegB");
		    PrintOctetComponent(comp);
		    //PrintFrameBuffer(frame);
		    printf("\n");
		}

	}

    SealFreeFrame(frame);
    FreeSeal(seal);

    return 0;
}
