#include <iostream>
#include <iomanip>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "seal/sealconfig.h"
#include "seal/sealscript.h"

using namespace std;

const int buflen = 255;

int main()
{
    int ret;
    SealScript *script;
    char buf[buflen] = { 0 };
    int linelen;

    script = LoadScript("aes.s");
    //script = LoadScript("aes2.s");

    //script->skipindent = true;
    for (int i = 0;
	 (SEAL_SCRIPT_EOF != (linelen = NextLine(script, buf, buflen))); i++)
	{
	    printf("%03d(%03d)\t:%s\n", i, linelen, buf);
	}

    cout << "******************************" << endl;

    for (int i = 0; i < 20; i++)
	{
	    if (0 < (ret = SyncLMark(script, "Mark1", buf, buflen)))
		cout << std::setfill('0') << std::
		    setw(3) << ret << '\t' << buf << endl;
	    else
		cout << "Error Code:" << ret << endl;
	}

    //Search for invalid Mark.
    ret = SyncLMark(script, "Mark2", buf, buflen);
    cout << "Mark2 Error Code:" << ret << endl;

    //Move Cursor only.
    ret = SyncLMark(script, "Mark1", NULL, buflen);
    cout << "Error Code:" << ret << endl;
    linelen = NextLine(script, buf, buflen);
    printf("%03d(%03d)\t:%s\n", 0, linelen, buf);

    ret = SyncLMark(script, "Mark2", NULL, buflen);
    cout << "Mark2 Error Code:" << ret << endl;

    FreeScript(script);
    return 0;
}
