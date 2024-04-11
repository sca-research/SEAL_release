#ifndef _CORE_H
#define _CORE_H

#include <stdlib.h>
#include <stdint.h>
#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif
//Types of Core Components.
    typedef enum _SealCoreComponentType_ {
	UNDEFINED,
	BOOL,
	OCTET,
	STRING,
	INT16,
	UINT16,
	INT32,
	UINT32
    } SealCoreComponentType;

//Union pointers for accessing Component Instances.
    typedef union {
	void *raw;
	uint8_t *BOOL;
	uint8_t *OCTET;
	char *STRING;
	int16_t *INT16;
	uint16_t *UINT16;
	int32_t *INT32;
	uint32_t *UINT32;
    } SealDataPointer;
    typedef SealDataPointer CompVal;

//Seal Core Component.
    typedef struct _SealCoreComponent_ {
	char *name;
	SealCoreComponentType type;
	size_t len;
    } SealCoreComponent;

//Get a string of Seal Core Component type.
    const char *SccGetTypeStr(SealCoreComponentType type);

//Get size of byte of a Seal Component type.
    int SccSizeOf(SealCoreComponentType type);

//Specification of a Core.
    typedef struct _SealCore_ {
	char *version;		//Core version.
	int32_t versionid;	//Internal ID for Core version.
	int ncomponents;	//Number of Core Components.
	SealCoreComponent **components;	//An array of Core Components.
    } SealCore;

//Constructor/Destructor a Seal Core.
    SealCore *ScNewCore();
    void ScDeleteCore(SealCore * sc);

//Load a Core Specification.
    SealCore *ScLoadCore(const char *sealcorepath);
#ifdef __cplusplus
}
#endif
#endif
