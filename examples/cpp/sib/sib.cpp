#include <iostream>
#include <iomanip>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "seal/sealconfig.h"
#include "seal/sealscript.h"

using namespace std;

const int BUFLEN = 255;

void Parser(int linenumber, const char *line, void *arg)
{
    int *retval = (int *)arg;
    printf("%d:\t%s\n", linenumber, line);
    retval[linenumber] = linenumber;
    return;
}

int main()
{
    SealScript *script;
    int ret;
    int i = 0;
    int pret[255] = { 0 };

    script = LoadScript("example_isw2.set");
    script->skipindent = true;

    while (true)
	{
	    printf("[%d]\n", i++);
	    ret = ProcessSib(script, Parser, (void *)pret);
	    if (SEAL_SCRIPT_EOF == ret)
		{
		    break;
		}
	    printf("Parser rets: ");
	    for (i = 0; i < ret; i++)
		{
		    printf("%d ", pret[i]);
		}
	    printf("\n");
	}

    FreeScript(script);
    return 0;
}
