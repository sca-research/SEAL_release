#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cjson/cJSON.h>

#include "seal/seal.h"

const char * TESTJSON="/tmp/testcorespec.scs";

using namespace std;


void PrintCoreInfo(SealCore * core)
{
    int i = 0;
    SealCoreComponent * scc = NULL;

    printf("Core version: %s\n", core->version);
        for(i = 0; i < core->ncomponents; i++)
    {
        scc = core->components[i];
        printf("Component [%02d]: %s\n", i, scc->name);
        printf("\t Type: %s\n", SccGetTypeStr(scc->type));
        printf("\t Length: %lu\n", scc->len);
    }

    return;
}

int main()
{
    SealCore * core = NULL;
    core = ScLoadCore(TESTJSON);
    PrintCoreInfo(core);
    ScDeleteCore(core);
    return 0;
}
