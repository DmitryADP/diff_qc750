/*
 * Copyright 2010 NVIDIA Corporation.  All Rights Reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation.  Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation
 * is strictly prohibited.
 */


#include "aos.h"
#include "nverror.h"

NvOdmDebugConsole
aosGetOdmDebugConsole(void)
{
    return NvOdmDebugConsole_Undefined;
}

NvError aosDebugInit(void)
{
    return NvSuccess;
}
NvBool aosDebugIsInitialized(void)
{
    return NV_FALSE;
}

void aosWriteDebugString(const char* str)
{
}

NvU32 aosDebugGetDefaultBaudRate(void)
{
    return 0;
}
