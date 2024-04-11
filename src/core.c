#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <cjson/cJSON.h>

#include "seal.h"
#include "core.h"
#include "sealutl.h"

#define STR_BOOL "BOOL"
#define STR_OCTET "OCTET"
#define STR_STRING "STRING"
#define STR_INT16 "INT16"
#define STR_UINT16 "UINT16"
#define STR_INT32 "INT32"
#define STR_UINT32 "UINT32"

static int CountJsonObj(cJSON * cjsonroot)
{
    int count = 0;
    cJSON *cj = NULL;

    cj = cjsonroot->child;
    while (NULL != cj)
	{
	    count++;
	    cj = cj->next;
	}

    return count;
}

//Create/delete a new Core Component.
SealCoreComponent *SccNewCoreComponent()
{
    SealCoreComponent *scc = NULL;
    scc = (SealCoreComponent *) malloc(sizeof(SealCoreComponent));
    scc->name = NULL;
    scc->type = OCTET;
    scc->len = 0;
    return scc;
}

void DeleteCoreComponent(SealCoreComponent * scc)
{
    if (NULL == scc)
	{
	    return;
	}
    Free(scc->name);
    scc->name = NULL;
    Free(scc);
    return;
}

//Convert a Component type string into a SealCoreComponentType.
SealCoreComponentType SccGetType(const char *typestr)
{
    if (!strncmp(typestr, STR_BOOL, SEAL_CORE_MAXSTRLEN))
	{
	    return BOOL;
	}
    else if (!strncmp(typestr, STR_OCTET, SEAL_CORE_MAXSTRLEN))
	{
	    return OCTET;
	}

    else if (!strncmp(typestr, STR_STRING, SEAL_CORE_MAXSTRLEN))
	{
	    return STRING;
	}
    else if (!strncmp(typestr, STR_INT16, SEAL_CORE_MAXSTRLEN))
	{
	    return INT16;
	}
    else if (!strncmp(typestr, STR_UINT16, SEAL_CORE_MAXSTRLEN))
	{
	    return UINT16;
	}

    else if (!strncmp(typestr, STR_INT32, SEAL_CORE_MAXSTRLEN))
	{
	    return INT32;
	}
    else if (!strncmp(typestr, STR_UINT32, SEAL_CORE_MAXSTRLEN))
	{
	    return UINT32;
	}
    return UNDEFINED;
}

//Return a string for the type of ${scc}.
const char *SccGetTypeStr(SealCoreComponentType type)
{
    const char *ret = NULL;

    switch (type)
	{
	case BOOL:
	    ret = STR_BOOL;
	    break;
	case OCTET:
	    ret = STR_OCTET;
	    break;
	case STRING:
	    ret = STR_STRING;
	    break;

	case INT16:
	    ret = STR_INT16;
	    break;
	case UINT16:
	    ret = STR_UINT16;
	    break;

	case INT32:
	    ret = STR_INT32;
	    break;
	case UINT32:
	    ret = STR_UINT32;
	    break;
	default:
	    ret = "UNDEFINED";
	    break;
	}

    return ret;
}

//Return size of each component type in bytes,
int SccSizeOf(SealCoreComponentType type)
{
    switch (type)
	{
	case BOOL:
	case OCTET:
	case STRING:
	    return 1;
	    break;
	case INT16:
	case UINT16:
	    return 2;
	    break;
	case INT32:
	case UINT32:
	    return 4;
	    break;
	default:
	    break;
	}
    return 0;
}

//Create/delete a SealCore object.
SealCore *ScNewCore()
{
    SealCore *sc = NULL;

    sc = (SealCore *) malloc(sizeof(SealCore));
    sc->version = NULL;
    sc->ncomponents = 0;
    sc->components = NULL;
    return sc;
}

void ScDeleteCore(SealCore * sc)
{
    int i;

    if (NULL == sc)
	{
	    return;
	}
    //Free version.
    Free(sc->version);

    //Free components.
    for (i = 0; i < sc->ncomponents; i++)
	{
	    DeleteCoreComponent(sc->components[i]);
	}
    Free(sc->components);

    //Free Core.
    Free(sc);
    return;
}

//Load a Seal Core ${sealcorepath}.
SealCore *ScLoadCore(const char *sealcorepath)
{
    int i;
    int jsonfd = -1;
    size_t jsonlen = 0;
    struct stat st = { 0 };
    char *jsonstr = NULL;
    SealCore *core = NULL;

    //cJSON pointers.
    cJSON *jsoncore = NULL;	//JSON entry for Core (i.e. CJSON root).
    cJSON *jsonversion = NULL;	//JSON entry for version.
    cJSON *jsoncomponents = NULL;	//JSON entry for components.
    cJSON *jsoncomp = NULL;	//JSON entry for each component.
    cJSON *jsoncompval = NULL;	//JSON entry for each component.
    SealCoreComponent *scc = NULL;	//Newly loaded Core object.

    //First read the JSON string from file.
    if (0 != stat(sealcorepath, &st))
	{
	    perror("stat() failed");
	    goto exit;
	}
    jsonlen = st.st_size;

    if (-1 == (jsonfd = open(sealcorepath, O_RDONLY)))
	{
	    perror("open() failed");
	    goto exit;
	}

    jsonstr = (char *)Malloc(jsonlen + 1);
    if (-1 == read(jsonfd, jsonstr, jsonlen))
	{
	    perror("read() failed");
	    goto exit;
	}
    //Convert a JSON string into a cJSON object.
    jsoncore = cJSON_Parse(jsonstr);

    //Convert the cJSON object into a SealCore object.
    core = ScNewCore();		//Initialise a new Core.
    //Extract "version".
    jsonversion = cJSON_GetObjectItemCaseSensitive(jsoncore, "version");
    core->version = CloneString(jsonversion->valuestring);
    core->versionid = atoi(core->version);
    //Count number of Components.
    jsoncomponents = cJSON_GetObjectItemCaseSensitive(jsoncore, "components");
    core->ncomponents = CountJsonObj(jsoncomponents);
    //Extract Components.
    //Allocate an array of pointers to SealCoreComponent objects.
    core->components = (SealCoreComponent **)
	Malloc(core->ncomponents * (sizeof(SealCoreComponent *)));
    //Convert each component into Component objects.
    jsoncomp = jsoncomponents->child;	//Initialise to first entry under "components".
    for (i = 0; i < core->ncomponents; i++, jsoncomp = jsoncomp->next)
	{
	    //Create a Core Component from a JSON entry.
	    scc = SccNewCoreComponent();
	    //Component name.
	    scc->name = CloneString(jsoncomp->string);
	    //Component type.
	    jsoncompval = cJSON_GetObjectItemCaseSensitive(jsoncomp, "type");
	    scc->type = SccGetType(jsoncompval->valuestring);
	    //Component length.
	    jsoncompval = cJSON_GetObjectItemCaseSensitive(jsoncomp, "len");
	    scc->len = jsoncompval->valueint;

	    //Store the new Core Component to Core.
	    core->components[i] = scc;
	}

 exit:
    //Clean.
    cJSON_Delete(jsoncore);
    Free(jsonstr);
    Close(jsonfd);
    return core;
}
