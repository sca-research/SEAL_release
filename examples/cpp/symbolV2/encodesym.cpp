#include "seal/seal.h"
#include "seal/symbolic.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void PrintCE(CodeEntry ce)
{
    printf("%d : %s\n", ce.symbolid, ce.symbolname);
    return;
}

void PrintDict(EncDict * dict)
{
    int count, i;
    CodeEntry *cea;

    count = EncDict2Array(&cea, dict);
    for (i = 0; i < count; i++)
	{
	    PrintCE(cea[i]);
	}
    free(cea);

    return;
}

int main()
{
    EncDict *dict, *imdict;
    CodeEntry *ce[3];

    ce[0] = (CodeEntry *) malloc(sizeof(CodeEntry));
    ce[1] = (CodeEntry *) malloc(sizeof(CodeEntry));
    ce[2] = (CodeEntry *) malloc(sizeof(CodeEntry));
    *ce[0] = { 1, strdup("111") };
    *ce[1] = { 2, strdup("2222") };
    *ce[2] = { 3, strdup("33333") };

    dict = NewEncDict();
    printf("Initiliased:\n");
    PrintDict(dict);

    AddCodeEntry(dict, ce[0]);
    AddCodeEntry(dict, ce[1]);
    AddCodeEntry(dict, ce[2]);

    printf("Entries added:\n");
    PrintDict(dict);

    DelEncDict(dict);

    printf("Importing a dictionary:\n");
    imdict = ImportEncDict("./test.dict");
    PrintDict(imdict);
    ExportEncDict(imdict, "/tmp/exported.dict");
    DelEncDict(imdict);

    return 0;
}
