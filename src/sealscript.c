#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

#include "seal.h"
#include "sealutl.h"

#include "sealscript.h"

//Load a script text.
SealScript *LoadScript(const char *scriptfile)
{
    SealScript *script;
    struct stat st = { 0 };

    script = (SealScript *) Malloc(sizeof(SealScript));

    //Open the script file read only.
    if (-1 == (script->fd = open(scriptfile, O_RDONLY)))
	{
	    perror("#open() failed:");
	    goto error;
	}

    //Set metadata for the script file.
    if (-1 == fstat(script->fd, &st))
	{
	    perror("#fstat() failed:");
	    goto error;
	}
    script->size = st.st_size;

    //Map the file into memory.
    if (MAP_FAILED ==
	(script->text =
	 (const char *)mmap(NULL, script->size, PROT_READ, MAP_PRIVATE,
			    script->fd, 0)))
	{
	    perror("#mmap() failed:");
	    goto error;
	}
    script->eof = script->text + script->size;

    //Configure Seal Mark style.
    script->lmarkstyle = SEAL_SCRIPT_LINE_MARK_STYLE;
    script->bmarkstyle = SEAL_SCRIPT_BLOCK_MARK_STYLE;

    //Configure Seal End of Line.
    script->eol = SEAL_SCRIPT_EOL;

    //Skip indent by default.
    script->skipindent = false;

    //Initialise the cursors.
    script->offset = 0;
    script->current = script->text;

    //Initialise the End of Block in a SIB.
    script->eobline = SEAL_SCRIPT_EOB;

    return script;

 error:
    FreeScript(script);
    return NULL;
}

//Free a SealScript.
void FreeScript(SealScript * script)
{
    munmap((void *)script->text, script->size);
    close(script->fd);
    Free(script);
    return;
}

#define DBG
#ifdef DBG
void Print(const char *from, const char *to)
{
    const char *c;
    for (c = from; c <= to; c++)
	{
	    printf("%c", *c);
	}

    return;
}
#endif

static inline int min(int x, int y)
{
    return (x < y ? x : y);
}

//Copy text into ${buf} from ${from} to ${to}, excluding ${to}.
static inline int TxtCpy(char *buf, const size_t buflen, const char *from,
			 const char *to)
{
    int cpylen;
    size_t txtlen = to - from;

    cpylen = min(txtlen, buflen - 1);
    memcpy(buf, from, cpylen);

    //Replace the last char by string terminator.
    buf[cpylen] = '\0';

    return cpylen;
}

//Reset Cursor of ${script}.
void ResetCursor(SealScript * script)
{
    script->current = script->text;
    script->offset = 0;
    return;
}

//Select the line pointed by ${cursor} and put it into ${buf}, from the beginning up to ${buflen} characters. 
//Returns the length of the line (excluding '\0').
int FetchLine(SealScript * script, const char *cursor, char *buf,
	      const size_t buflen)
{
    const char *const text = script->text;
    const char *const textend = script->eof;
    const char *lstart, *lend;	//Start and End of line.

    //Find the start of line.
    for (lstart = cursor - 1; (lstart >= text) && (script->eol != *lstart);
	 lstart--) ;
    lstart++;

    //Find the end of line.
    for (lend = cursor; (lend <= textend) && (script->eol != *lend); lend++) ;
    if (lend == textend)
	{
	    lend++;
	}

    //Skip the indent.
    if (script->skipindent)
	{
	    for (; ' ' == *lstart || '\t' == *lstart; lstart++) ;
	}

    //Copy the line and return its length.
    return TxtCpy(buf, buflen, lstart, lend);
}

//Copy the line at Cursor from beginning into ${line} upto ${linelen} characters. A '\0' is guaranteed at the end of ${line}. 
//The Cursor is updated to the begining of the line after.
//Returns the length of the copied line.
int NextLine(SealScript * script, char *line, const size_t linelen)
{
    int retlen = 0;
    const char *cursor = script->current;

    //Reset if Cursor is at EOF.
    if (script->eof <= cursor)
	{
	    ResetCursor(script);
	    return SEAL_SCRIPT_EOF;
	}

    //Copy the currnet line.
    retlen = FetchLine(script, cursor, line, linelen);

    //Move Cursor to the next line.
    for (; (script->eof >= cursor) && (script->eol != *cursor); cursor++) ;
    if (script->eol == *cursor)
	{
	    cursor++;
	}

    script->current = cursor;
    script->offset = script->current - script->text;

    return retlen;
}

//Synchronise to Line Marker given by ${markname} and fetch that line into ${buf}. The Seal Cursor is updated to after the Mark. The Cursor will be reset in case ${markname} is not found.
//If ${buf} is NULL then no line will be fetched and only the Cursor is updated. ${buflen} is ignored in this case. The return value is also fixed to SEAL_SUCCESS (0), or SEAL_SCRIPT_MARK_NOT_FOUND (-2) if the mark is not found.
int SyncLMark(SealScript * script, const char *markname, char *buf,
	      const size_t buflen)
{
    int linelen = 0;
    int marklen = 0;
    const char *markpos = NULL;
    char mark[SEAL_SCRIPT_MARK_LEN] = { 0 };

    //Construct the Mark string.
    marklen = snprintf(mark, sizeof(mark) - 1, script->lmarkstyle, markname);

    //Find the Mark in the text.
    if (NULL == (markpos = strstr(script->current, mark)))
	{
	    //Mark not found.
	    ResetCursor(script);
	    return SEAL_SCRIPT_MARK_NOT_FOUND;
	}

    //Fill the buf with the marked line if buf is not NULL.
    if (NULL != buf)
	{
	    //Reset buf.
	    memset(buf, 0, buflen);
	    linelen = FetchLine(script, markpos, buf, buflen);
	}

    //Update cursor.
    script->current = markpos + marklen;
    script->offset = script->current - script->text;
    if (script->offset >= script->size)
	{
	    //Reset after reaching EoF.
	    ResetCursor(script);
	}

    return linelen;
}

//Process the next Seal Instruction Block in ${script} by ${lp}.
int ProcessSib(SealScript * script, SibSed lp, void *lparg)
{
    int linelen;
    char buf[SEAL_SCRIPT_MAX_LINE_LEN] = { 0 };
    int linecount = 0;

    while (true)
	{
	    //Read the next line in from current position.
	    linelen = NextLine(script, buf, sizeof(buf));

	    if (SEAL_SCRIPT_EOF == linelen)
		{		//Sib reached EOF.
		    return SEAL_SCRIPT_EOF;
		}
	    else if (0 == strncmp(script->eobline, buf, sizeof(buf)))	//Terminator.
		{		//Sib reached EOB.
		    break;
		}

	    //A valid line.
	    if (NULL != lp)
		{
		    lp(linecount, buf, lparg);	//Call line parser.
		}
	    linecount++;
	}

    return SEAL_SUCCESS;
}

//Initialise a Seal Queue.
SealQueue *NewSealQueue()
{
    SealQueue *sq = NULL;

    sq = (SealQueue *) Malloc(sizeof(SealQueue));
    sq->count = 0;
    sq->queue = NULL;
    sq->lasthint = NULL;

    return sq;
}

//Delete a Seal Queue.
void DelSealQueue(SealQueue * sq, FreeFunc FF)
{
    DelChain(&sq->queue, FF);
    Free(sq);
    return;
}

////Push an object into the queue. 
//Returns:
//  The number of elements in the queue after push.
int SqPush(SealQueue * sq, void *obj)
{
    //Add obj into the queue.
    sq->lasthint = AddNode(&sq->queue, sq->lasthint, obj);
    sq->count++;
    return sq->count;
}

//Pop the oldest object from the queue.
//Returns:
//  The popped element, or NULL if the queue is empty,
void *SqPop(SealQueue * sq)
{
    void *obj = NULL;

    if (NULL == sq->queue || NULL == sq)
	{
	    //No element in the queue.
	    return NULL;
	}

    //Set return to be the contents of first element in the queue.
    obj = sq->queue->content;

    //Remove the first element. No content free is needed.
    DelNode(&sq->queue, sq->queue, NULL);
    sq->count--;

    return obj;
}
