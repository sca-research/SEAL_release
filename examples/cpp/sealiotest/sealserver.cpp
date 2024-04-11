#include "seal/seal.h"
#include "seal/emulator.h"

#include <stdio.h>
#include <iostream>

using namespace std;

const char *TESTIOPATH = "/tmp/testio";

int main()
{
	SealIO *sio = NULL;

	sio = SioOpen(TESTIOPATH);

	while (true) {
		int ret;

		printf("Waiting for connection...");
		fflush(stdout);
		if (SEAL_ERROR == (ret = SioWaitConn(sio))) {
			printf("Error.\n");
			break;
		}
		printf(" established.\n");

		while (true) {
			int c;

			c = SioGetchar(sio);
			if (SEAL_ERROR == c) {
				if (SEAL_IO_EOF == sio->stat) {
					printf("Connection closed.\n");
				} else {
					printf("SealIO error.\n");
				}
				break;
			}
			printf("Received: %c\n", c);

			if (SEAL_ERROR == SioPutchar(sio, c)) {
				printf("SealIO error.\n");
				break;
			}
			printf("Send: %c\n", c);
		}
	}

	SioClose(sio);

	return 0;
}
