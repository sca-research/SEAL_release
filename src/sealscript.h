#ifndef _SEALSCRIPT_H
#define _SEALSCRIPT_H

#include <stdlib.h>
#include <stdbool.h>

#include "sealconfig.h"
#include "chain.h"

#define SEAL_SCRIPT_BADFILE (-1)
#define SEAL_SCRIPT_EOF (-1)
#define SEAL_SCRIPT_MARK_NOT_FOUND (-2)

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct __SealScript {
	int fd;
	const char *text;
	const char *eof;
	size_t size;
	unsigned int offset;
	const char *current;
	const char *lmarkstyle;	//Seal Line Mark Style string.
	const char *bmarkstyle;	//Seal Block Mark Style string.
	char eol;		//End of line.
	bool skipindent;
	const char *eobline;	//End of block line in a SIB.
    } SealScript;
    //Load a script text.
    SealScript *LoadScript(const char *scriptfile);
    //Free a SealScript.
    void FreeScript(SealScript * script);

    //Reset Cursor of ${script}.
    void ResetCursor(SealScript * script);

    //Select the line pointed by ${cursor} and put it into ${buf}, from the beginning up to ${buflen} characters.
    //Returns the length of the line (excluding '\0').
    int FetchLine(SealScript * script, const char *cursor, char *line,
		  const size_t linelen);

    //Copy the next line at Cursor into ${line} upto ${linelen} characters. A '\0' is guaranteed at the end of ${line}. The Cursor is updated to the begining of the line after.
    //Returns the length of the copied line.
    int NextLine(SealScript * script, char *line, const size_t linelen);

    //Synchronise to Line Marker given by ${markname} and fetch that line into ${buf}. The Seal Cursor is updated to after the Mark. The Cursor will be reset in case ${markname} is not found.
    //If ${buf} is NULL then no line will be fetched and only the Cursor is updated. ${buflen} is ignored in this case. The return value is also fixed to SEAL_SUCCESS (0), or SEAL_SCRIPT_MARK_NOT_FOUND (-2) if the mark is not found.
    int SyncLMark(SealScript * script, const char *markname, char *buf,
		  const size_t buflen);

    //Prototype for Sib line parser.
    //${linenumber} : Counter of lines from 0 in SIB.
    //${line}   : The current line in SIB terminated by '\0'.
    //${arg}: An argument shared by each call of SibAwk.
    typedef void (*SibSed)(int linenumber, const char *line, void *arg);

    //Process the next Seal Instruction Block (SIB) in ${script} with ${lp}.
    int ProcessSib(SealScript * script, SibSed lp, void *lparg);

    //Queue for asynchrnous execution of Seal Instructions.
    //A queue works as an FIFO, that is, pop always returns the oldest element.
    typedef struct {
	int count;
	ChainNode *queue;
	ChainNode *lasthint;
    } SealQueue;
    SealQueue *NewSealQueue();
    void DelSealQueue(SealQueue * sq, FreeFunc FF);

    //Push an object into the queue. 
    //Returns:
    //  The number of elements in the queue after push.
    int SqPush(SealQueue * sq, void *obj);

    //Pop the oldest object from the queue.
    //Returns:
    //  The popped element, or NULL if the queue is empty,
    void *SqPop(SealQueue * sq);

#ifdef __cplusplus
}
#endif
#endif
