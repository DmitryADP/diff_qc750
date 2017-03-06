/**
 * Copyright (c) 2007-2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

/*
 * nvbuildbct.h - Definitions for the nvbuildbct code.
 */

/*
 * TODO / Notes
 * - Add doxygen commentary
 */

#ifndef INCLUDED_NVBUILDBCT_H
#define INCLUDED_NVBUILDBCT_H

#include "nvbuildbct_config.h"
#include "nvos.h"


#include <stdlib.h> /* For definitions of exit(), rand(), srand() */

#if defined(__cplusplus)
extern "C"
{
#endif

/*
 * Forward declarations
 */
struct NvBootImgRec;

/*
 * Enumerations
 */

/* Not used yet. For future usage. */
typedef enum
{
    BctMode_None = 0,
    BctMode_OneBct,
    BctMode_Force32 = 0x7fffffff
} BctMode;

typedef enum
{
    SecurityMode_None = 0,
    SecurityMode_Plaintext,
    SecurityMode_Checksum,
    SecurityMode_Encrypted,
    SecurityMode_Max,
    SecurityMode_Force32 = 0x7fffffff
} SecurityMode;

typedef enum
{
    RandomAesBlockType_None = 0,
    RandomAesBlockType_Zeroes,
    RandomAesBlockType_Literal,
    RandomAesBlockType_Random,
    RandomAesBlockType_RandomFixed,
    RandomAesBlockType_Force32 = 0x7fffffff
} RandomAesBlockType;


/// Defines various data widths supported by Emmc.

typedef struct BuildBctContextRec
{
    NvOsFileHandle  ConfigFile;
    char  CfgFilename[MAX_BUFFER];
    char  ImageFilename[MAX_BUFFER];

    NvOsFileHandle       RawFile;
    struct NvBootImgRec *OutputImg;

    RandomAesBlockType       RandomBlockType;
    NvU8               RandomBlock[CMAC_HASH_LENGTH_BYTES];

    NvU32  BlockSize;
    NvU32  BlockSizeLog2;

    NvU32  PageSize;
    NvU32  PageSizeLog2;
    NvU32  PagesPerBlock;

    NvU32  PartitionSize;
    NvU32  Redundancy;

    NvU32  Version;
    NvBool VersionSetBeforeBL; /* True when set ahead of a bootloader.
                                * When false, Version will autoincrement
                                * with each bootloader.
                                */

    /* Allocation data. */
    struct BlockDataRec *Memory; /* Representation of memory */

    NvBool BctWritten;           /* Initially NV_FALSE */

    NvU8  NewBlFilename[MAX_BUFFER];
    NvU32  NewBlLoadAddress;
    NvU32  NewBlEntryPoint;
    NvU32  NewBlStartBlk;
    NvU32  NewBlStartPg;
    NvU32  NewBlAttribute;

    NvBool LogMemoryOps; /* Defaults to false.  True after dumping image. */

    NvU8   *NvBCT;
    NvU32 NvBCTLength;   /* Since different chips might have a different
                          * bct, the length may vary and needs to be a
                          * part of the BuildBctContext
                          */
} BuildBctContext;


NvError OpenFile(const char *Filename, NvU32 Flags, NvOsFileHandle *File);

NvError WriteImageFile(BuildBctContext *Context);

/*
 * Creates a bctfile using the parsing code  based on the input arguments
 *
 *.@param CfgFile Input configfile
 *.@param BctFIle Optional output bct filename given by the user
 */
NvError NvBuildBct(const char *CfgFile, const char *BctFile, NvU32 ChipId);

/* Global data */
extern NvBool EnableDebug;

#if defined(__cplusplus)
}
#endif

#endif /* #ifndef INCLUDED_NVBUILDBCT_H */
