/*
 * Copyright (c) 2005-2007 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef INCLUDED_TIO_SDRAM_H
#define INCLUDED_TIO_SDRAM_H

//###########################################################################
//############################### TYPEDEFS ##################################
//###########################################################################

//===========================================================================
// max size of NvBootSdramParams of all supported projects in 32bit-words
//===========================================================================
enum { NV_TIO_MAX_SDRAM_DATA_BUFFER_LEN = 256 };

//===========================================================================
// NvTioSdramData - Representation of SDRAM data passed from the host
//===========================================================================
typedef struct NvTioSdramDataRec
{
    NvU32             Magic;   /* Identify struct in binary data */
    NvU32             Version; /* Identify version of structure  */
    NvU32             Params[NV_TIO_MAX_SDRAM_DATA_BUFFER_LEN];  /* parameter storage */
} NvTioSdramData;


//===========================================================================
// Sdram Package version number
//===========================================================================
typedef enum
{
    NvTioSdramVersion_None    = 0x00000001,
    NvTioSdramVersion_Ap15    = 0x10001000,
    NvTioSdramVersion_Ap20    = 0x20001000,
    NvTioSdramVersion_T30     = 0x30001000,
    NvTioSdramVersion_Current = NvTioSdramVersion_T30,
    NvTioSdramVersion_Force32 = 0x7FFFFFFF
} NvTioSdramVersion;

//===========================================================================
// Sdram Package indicator
//===========================================================================
enum { NVTIO_SDRAM_MAGIC = 0x5244446E };

#endif // INCLUDED_TIO_SDRAM_H
