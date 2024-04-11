#include "print.hpp"
#include "seal/seal.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <iostream>

using namespace std;

void PrintFrame(const SealFrame * frame, const SealCore * core)
{
    int i;
    const int LINEWIDTH = 16;

    printf("Frame mapping:");
    for (i = 0; i < frame->len; i++)
	{
	    if (!(i % LINEWIDTH))
		printf("\n[%02d]: ", i);
	    printf("%02X ", frame->buf[i]);
	}
    cout << endl;
    return;
}

void PrintComponent(const SealFrameCompInst * idx, const SealCore * core)
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

    //Version 2 only.
    if (2 == core->versionid)
	{
	    printf(" Symbols: [");
	    if (STRING == idx->type)
		{
		    printf("%08X", ntohl(idx->symid[0]));
		}
	    else
		{
		    for (i = 0; i < idx->len; i++)
			{
			    printf("%08X", ntohl(idx->symid[i]));
			    if (i != idx->len - 1)
				{
				    printf(" ");
				}
			}
		}
	    printf("]");
	}
    printf("\n");
    return;
}
