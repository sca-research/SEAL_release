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
#include <wait.h>

#include <cjson/cJSON.h>

#include "seal/seal.h"

#define INTERACTIVE (0)

const char *TESTJSON = "testcore.scs";
const char *ITA_TEST_TRACE = "/tmp/sealtraceif";

using namespace std;

void PrintFrame(SealTraceFrame * frame)
{
	int i;

	printf("Frame mapping:");
	for (i = 0; i < frame->len; i++) {
		if (!(i % 8))
			printf("\n[%02d]: ", i);

		printf("%02X ", frame->buf[i]);
	}
	cout << endl;
	return;
}

void PrintComponent(SealTraceFrameIdx * idx)
{
	int i;

	printf("|%-12s|%-8s|%02d| = [%012lX] ",
	       idx->name, SccGetTypeStr(idx->type), idx->len, (long)idx->val);
	for (i = 0; i < (SccSizeOf(idx->type) * idx->len); i++) {
		printf("%02X ", ((unsigned char *)idx->val)[i]);
	}

	if (OCTET != idx->type) {
		printf("(");
		//Translated info.
		if (STRING == idx->type) {
			printf("%s", (const char *)idx->val);
		} else {
			for (i = 0; i < idx->len; i++) {
				switch (idx->type) {
				case INT32:
					printf("%d", ntohl(((const int32_t *)
							    idx->val)[i]));
					break;

				case UINT32:
					printf("%d", ntohl(((const uint32_t *)
							    idx->val)[i]));
					break;

				default:
					break;
				}

				if (idx->len - 1 > i)
					printf(",");
			}
		}
		printf(")");
	}
	printf("\n");
	return;
}

void ReadRecord(SealCore * core)
{
	SealTraceFrame *frame = NULL;
	SealTraceFrameIdx rega, regb, exeins, mode, cyclecount;
	SealTrace *trace = NULL;
	int cycle, ret, pidstat;
	pid_t pid;

	mkfifo(ITA_TEST_TRACE, S_IWUSR | S_IRUSR);
	if (0 == (pid = fork())) {
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
		exit(system("./gentrace /tmp/sealtraceif"));
	}
	//Init
	trace = StOpen(ITA_TEST_TRACE, core, SEAL_TRACE_MODE_READ);
	frame = StNewFrame(core);

	//Set up index to components.
	StfGetFrameIndex(frame, &rega, "RegA");
	StfGetFrameIndex(frame, &regb, "RegB");
	StfGetFrameIndex(frame, &exeins, "ExeIns");
	StfGetFrameIndex(frame, &mode, "Mode");
	StfGetFrameIndex(frame, &cyclecount, "CycleCount");

	for (cycle = 0; 0 < (ret = StNextFrame(trace, frame)) && cycle <= 11;
	     cycle++) {
		//Print each Core Component.
		printf("===== CYCLE: %02d =====\n", cycle);
		printf("Frame ID: %ld\n", trace->offset);
		PrintComponent(&rega);
		PrintComponent(&regb);
		PrintComponent(&exeins);
		PrintComponent(&mode);
		PrintComponent(&cyclecount);
		PrintFrame(frame);
		printf("=====************=====\n\n");
	}

	StClose(trace);
	StDeleteFrame(frame);
	wait(&pidstat);
	return;
}

int main(int argc, char *argv[])
{
	SealCore *core = NULL;

	//Read a Core.
	core = ScLoadCore(TESTJSON);

	//Read Trace file.
	ReadRecord(core);

	ScDeleteCore(core);
	unlink(ITA_TEST_TRACE);
	return 0;
}
