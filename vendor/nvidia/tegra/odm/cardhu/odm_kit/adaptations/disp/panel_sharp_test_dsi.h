/*
 * Copyright (c) 2012, NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software and related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef INCLUDED_PANEL_SHARP_DSI_H
#define INCLUDED_PANEL_SHARP_DSI_H

#include "panels.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define SHARP_TEST_DSI_GUID NV_ODM_GUID('s','s','t','s','t','d','s','i')

void sharpTestDsi_GetHal(NvOdmDispDeviceHandle hDevice);

#if defined(__cplusplus)
}
#endif

#endif
