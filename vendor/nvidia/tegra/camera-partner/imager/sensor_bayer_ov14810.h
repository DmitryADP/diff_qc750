/**
 * Copyright (c) 2011 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software and related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef SENSOR_BAYER_OV14810_H
#define SENSOR_BAYER_OV14810_H

#include "imager_hal.h"
#include "nvodm_query_discovery_imager.h"

#if defined(__cplusplus)
extern "C"
{
#endif

NvBool SensorBayerOV14810_GetHal(NvOdmImagerHandle hImager);

#if defined(__cplusplus)
}
#endif

#endif  //SENSOR_BAYER_OV14810_H
