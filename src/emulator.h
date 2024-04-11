#include "seal.h"
#include "sealconfig.h"

#include <stdint.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifndef _EMULATOR_H
#define _EMULATOR_H

#define SEAL_IO_WAITCONN (0)
#define SEAL_IO_READY (1)
#define SEAL_IO_EOF (-2)
#define SEAL_IO_ERROR (0xFF)
#define SEAL_IO_SERVER (0)
#define SEAL_IO_CLIENT (1)

#ifdef __cplusplus
extern "C" {
#endif
    //APIs for Emulator.
    //Bind a variable.
    int SealBind(Seal * seal, const char *comp, const void *var);

    //Sync all bound variables.
    void SealSync(Seal * seal);

    //APIs for SealIO.
    //Data structure for SealIO.
    typedef struct _SealIO_ {
	struct sockaddr_un addr;
	int sockfd;		//FD for socket.
	int commfd;		//FD for communication.
	int stat;		//State of SealIO.
	int type;
    } SealIO;

    //Initiate a new SealIO.
    SealIO *SioOpen(const char *);
    void SioClose(SealIO *);
    SealIO *SioConnect(const char *path);
    int SioWaitConn(SealIO *);

    //Basic IO.
    int SioGetchar(SealIO *);
    int SioPutchar(SealIO *, const int);
#ifdef __cplusplus
}
#endif
#endif
