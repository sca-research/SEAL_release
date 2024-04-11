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

void PrintPopval(SealQueue * sq)
{
    int *ip = NULL;

    ip = (int *)SqPop(sq);

    if (NULL == ip)
	{
	    printf("Empty queue.\n");
	}
    else
	{
	    printf("%d\n", *ip);
	}

    return;
}

int main()
{
    SealQueue *sq;

    int intvals[] = { 1, 2, 3, 4, 5 };

    sq = NewSealQueue();	//[]

    SqPush(sq, &intvals[0]);	//[1]
    SqPush(sq, &intvals[1]);	//[1 2]
    SqPush(sq, &intvals[2]);	//[1 2 3]

    PrintPopval(sq);		//1 [2 3]
    PrintPopval(sq);		//2 [3]

    SqPush(sq, &intvals[3]);	//[3 4]
    SqPush(sq, &intvals[3]);	//[3 4 4]

    PrintPopval(sq);		//3 [4 4]
    PrintPopval(sq);		//4 [4]

    SqPush(sq, &intvals[4]);	//[4 5]

    PrintPopval(sq);		//4 [5]

    SqPush(sq, &intvals[1]);	//[5 2]
    SqPush(sq, &intvals[0]);	//[5 2 1]

    PrintPopval(sq);		//5 [2 1]
    PrintPopval(sq);		//2 [1]
    PrintPopval(sq);		//1 []

    SqPush(sq, &intvals[0]);	//[1]
    SqPush(sq, &intvals[1]);	//[1 2]
    SqPush(sq, &intvals[2]);	//[1 2 3]

    PrintPopval(sq);		//1 [2 3]
    PrintPopval(sq);		//2 [3]
    PrintPopval(sq);		//3 []
    PrintPopval(sq);		//E []
    PrintPopval(sq);		//E []

    DelSealQueue(sq, NULL);

    return 0;
}
