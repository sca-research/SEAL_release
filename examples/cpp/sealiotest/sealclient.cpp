#include "seal/seal.h"
#include "seal/emulator.h"

#include <stdio.h>
#include <iostream>

using namespace std;

const char *TESTIOPATH = "/tmp/testio";

int main()
{
	int c;
	SealIO *sio;

	sio = SioConnect(TESTIOPATH);

	while ('\0' != (c = getchar())) {
		if ('\n' == c) {
			continue;
		}
		SioPutchar(sio, c);
		printf("%c\n", SioGetchar(sio));
	}

	SioClose(sio);
	return 0;
}
