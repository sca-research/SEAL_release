//Seal headers.
#include "seal.h"
#include "emulator.h"
#include "sealconfig.h"

//C headers.
#include <stdio.h>
#include <string.h>

//Unix headers.
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

//APIs for Emulator.
//Bind a variable to the SEAL object.
int SealBind(Seal * seal, const char *comp, const void *var)
{
    ValMap *currmap, *lastmap = NULL;
    SealFrameCompInst *ci = NULL;

    //Traverse the mapping entries.
    for (currmap = seal->maps; NULL != currmap;
	 lastmap = currmap, currmap = currmap->next)
	{
	    if (0 == strncmp(comp, currmap->ci->name, SEAL_CORE_MAXSTRLEN))	//A mapping exists.
		{
		    //Update the existing mapping entry.
		    currmap->emuvar = var;
		    return SEAL_SUCCESS;
		}
	}

    //Retrieve a new Seal Trace Frame Index.
    ci = (SealFrameCompInst *) malloc(sizeof(SealFrameCompInst));
    if (SEAL_SUCCESS != SfFetchFrameCI(seal->frame, ci, comp))
	{
	    INFO("#Component \"");
	    INFO(comp);
	    INFO("\" not found\n");
	    free(ci);
	    return SEAL_ERROR;
	}

    if (NULL == seal->maps)	//Initialise the mapping.
	{
	    seal->maps = (ValMap *) malloc(sizeof(ValMap));
	    seal->maps->ci = ci;
	    seal->maps->emuvar = var;
	    seal->maps->next = NULL;
	    return SEAL_SUCCESS;
	}

    //No existing mapping found. Create a new entry in the mappings.
    lastmap->next = (ValMap *) malloc(sizeof(ValMap));
    lastmap->next->ci = ci;
    lastmap->next->emuvar = var;
    lastmap->next->next = NULL;

    return SEAL_SUCCESS;
}

//Sync all binded variables.
void SealSync(Seal * seal)
{
    ValMap *map;

    for (map = seal->maps; NULL != map; map = map->next)
	{
	    SfAssign(map->ci, map->emuvar);
	}

    return;
}

//Init a new SealIO specified by ${path}.
//Return:
//  On success, a pointer to initiated SealIO object.
//  On failure a NULL pointer is returned.
SealIO *SioOpen(const char *path)
{
    SealIO *sio = NULL;

    //Memory allocation.
    sio = (SealIO *) malloc(sizeof(SealIO));
    memset(sio, 0, sizeof(SealIO));

    //Clear previous SealIO.
    unlink(path);

    //Initiate a streaming UNIX domain server.
    if (-1 == (sio->sockfd = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
	    perror("#socket() failed");
	    free(sio);
	    return NULL;
	}
    INFO("#Socket initiated at:");
    INFO(path);
    INFO("\n");

    //Set UNIX domain URL.
    sio->addr.sun_family = AF_UNIX;
    strncpy(sio->addr.sun_path, path, SEAL_IO_MAXPATHLEN);
    if (-1 ==
	bind(sio->sockfd, (struct sockaddr *)&sio->addr,
	     sizeof(struct sockaddr_un)))
	{
	    perror("#bind() failed");
	    free(sio);
	    return NULL;
	}
    INFO("#Socket bound.\n");

    if (-1 == listen(sio->sockfd, 1))
	{
	    perror("#listen() failed");
	    free(sio);
	    return NULL;
	}
    INFO("#Socket listen.\n");

    sio->stat = SEAL_IO_WAITCONN;
    sio->type = SEAL_IO_SERVER;

    return sio;
}

//Close a SealIO.
//Return:
//  This function is assumed to be always success and thus does not return.
void SioClose(SealIO * sio)
{
    close(sio->sockfd);
    close(sio->commfd);
    if (SEAL_IO_SERVER == sio->type)
	{			//Only unlink the path when this is a server.
	    unlink(sio->addr.sun_path);
	}
    free(sio);
    INFO("#SealIO closed.\n");
    return;
}

//Connect to a SealIO.
//Return:
//  On success it returns an SealIO object connected, otherwise it returns a NULL pointer.
SealIO *SioConnect(const char *siopath)
{
    SealIO *sio;

    //Initialise the SealIO clinet.
    sio = (SealIO *) malloc(sizeof(SealIO));
    memset(sio, 0, sizeof(SealIO));
    if (-1 == (sio->sockfd = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
	    perror("#socket() failed");
	    free(sio);
	    return NULL;
	}
    //Set UNIX domain URL.
    sio->addr.sun_family = AF_UNIX;
    strncpy(sio->addr.sun_path, siopath, SEAL_IO_MAXPATHLEN);

    //Connect to the existing SealIO interface.
    if (-1 ==
	connect(sio->sockfd, (struct sockaddr *)&sio->addr,
		sizeof(struct sockaddr_un)))
	{
	    perror("#connect() failed");
	    free(sio);
	    return NULL;
	}
    sio->commfd = sio->sockfd;
    INFO("#socked connected\n");

    sio->stat = SEAL_IO_READY;
    sio->type = SEAL_IO_CLIENT;

    return sio;
}

//Wait for a SealIO connection.
int SioWaitConn(SealIO * sio)
{
    if (SEAL_IO_READY == sio->stat)
	{
	    INFO("#Connection established.\n");
	    return SEAL_SUCCESS;
	}

    if (-1 == (sio->commfd = accept(sio->sockfd, NULL, 0)))
	{
	    perror("accept() failed");
	    return SEAL_ERROR;
	}
    sio->stat = SEAL_IO_READY;
    INFO("#Connection accepted.\n");

    return SEAL_SUCCESS;
}

//Read a byte from SealIO {sio}.
//Return:
//  On success returns unsigned char (converted to int). 
//  On failure or EOF, SEAL_ERROR is returned and stat is set.
int SioGetchar(SealIO * sio)
{
    int ret = 0;
    unsigned char c;
    char infomsg[SEAL_INFO_MAXLEN] = { 0 };

    if (sio->stat != SEAL_IO_READY)
	{
	    INFO("#SealIO not ready.\n");
	    return SEAL_ERROR;
	}
    //Receive a byte.
    ret = recv(sio->commfd, &c, 1, 0);
    if (0 >= ret)
	{
	    if (0 == ret)
		{		//Connection closed from remote.
		    INFO("#SealIO closed by remote.\n");
		    sio->stat = SEAL_IO_EOF;
		}
	    else
		{		//Syscall failure.
		    perror("#recv() failed");
		    sio->stat = SEAL_IO_ERROR;
		}
	    close(sio->commfd);
	    return SEAL_ERROR;
	}
    snprintf(infomsg, sizeof(infomsg), "#Received: %03d\n", (unsigned int)c);
    INFO(infomsg);

    return (int)c;
}

//Write a byte to SealIO {sio}.
//Return:
//  On success returns SEAL_SUCCESS.
//  On failure or EOF, SEAL_ERROR is returned and stat is set.
int SioPutchar(SealIO * sio, const int charval)
{
    int ret;
    char infomsg[SEAL_INFO_MAXLEN] = { 0 };
    unsigned char c = (char)(0xFF & charval);

    snprintf(infomsg, sizeof(infomsg), "#Send: %03d\n", (unsigned int)c);
    INFO(infomsg);
    ret = send(sio->commfd, &c, 1, MSG_NOSIGNAL);
    if (0 >= ret)
	{
	    if (0 == ret)
		{
		    INFO("SealIO closed by remote.\n");
		    sio->stat = SEAL_IO_EOF;
		}
	    else
		{
		    perror("#send() failed");
		    sio->stat = SEAL_IO_ERROR;
		}
	    return SEAL_ERROR;
	}

    return c;
}
