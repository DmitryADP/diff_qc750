/*
 * Copyright (c) 2011-2012, NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */
#define NV_ENABLE_DEBUG_PRINTS 0

#if (BUILD_FOR_AOS == 0)
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <asm/types.h>
#include <stdint.h>
#include <ar0832_main.h>
#include "sensor_focuser_ar0832_common.h"
#endif

#include "focuser_ar0832.h"
#include "imager_util.h"
#include "nvassert.h"

// enable the real assert even in release mode
#undef NV_ASSERT
#define NV_ASSERT(x) \
    do{if(!(x)){NvOsDebugPrintf("ASSERT at %s:%d\n", __FILE__,__LINE__);}}while(0)

#define LENS_FOCAL_LENGTH   3.5f
#define LENS_FNUMBER        2.6f
#define LENS_MAX_APERTURE   1.346f


#define GRAVITY_FUDGE(x) (85 * (x) / 100)

#define TIME_DIFF(_from, _to) \
    (((_from) <= (_to)) ? ((_to) - (_from)) : \
     (~0UL - ((_from) - (_to)) + 1))

/**
*Focuser's context
*/
typedef struct {
    int focuser_fd;
    NvOdmImagerPowerLevel PowerLevel;
    NvU32 cmdTime;                // time of latest focus command issued
    NvU32 Position;                // The last settled focus position.
    NvU32 RequestedPosition;         // The last requested focus position.
    NvS32 DelayedFPos;            // Focusing position for delayed request
    NvF32 FocalLength;
    NvF32 FNumber;
    NvF32 MaxAperture;
#if (BUILD_FOR_AOS == 0)
    struct nv_focuser_config Config;
    AR0832_Stereo_Info *pStereo_Info;
#endif
} FocuserCtxt;


static void Focuser_UpdateSettledPosition(FocuserCtxt *ctxt)
{
#if (BUILD_FOR_AOS == 0)
    NvU32 CurrentTime = 0;
#endif

    // Settled position has been updated?
    if (ctxt->Position == ctxt->RequestedPosition)
    {
        NV_DEBUG_PRINTF(("[%s] position = RequestedPosition = %d\n",
                        __func__, ctxt->Position));
        return;
    }

#if (BUILD_FOR_AOS == 0)
    CurrentTime = NvOdmOsGetTimeMS();

    // Previous requested position settled?
    if (TIME_DIFF(ctxt->cmdTime, CurrentTime) >= ctxt->Config.settle_time)
    {
        ctxt->Position = ctxt->RequestedPosition;
        NV_DEBUG_PRINTF(("[%s] timeDiff %d >= setlleTime %d Position %d\n",
                        __func__, TIME_DIFF(ctxt->cmdTime, CurrentTime),
                        ctxt->Config.settle_time, ctxt->Position));
    }
#endif
}

/**
*setPosition.
*/
#if (BUILD_FOR_AOS == 0)
static NvBool setPosition(FocuserCtxt *pContext, NvS32 Position)
{

    if (Position < pContext->Config.pos_actual_low && Position > pContext->Config.pos_actual_high)
    {
        NV_DEBUG_PRINTF(("%s: position %d out of bounds\n", __func__, Position));
        return NV_FALSE;
    }
    NV_DEBUG_PRINTF(("%s: position = %d\n", __func__, Position));

    if (pContext->pStereo_Info->drv_ioctl(
            pContext->pStereo_Info, AR0832_FOCUSER_IOCTL_SET_POSITION,
            (void *)(Position)) < 0)
    {
        NV_DEBUG_PRINTF(("%s: ioctl to set focus failed - %s\n",
                        __FILE__, strerror(errno)));
        return NV_FALSE;
    }
    pContext->cmdTime = NvOdmOsGetTimeMS();
    pContext->RequestedPosition = Position;
    NV_DEBUG_PRINTF(("[%s] position = %d. Sleeping for 10 ms\n",
                        __func__, Position));
    NvOdmOsSleepMS(10);
    return NV_TRUE;
}
#endif

/**
*Focuser_Close
*Free focuser's context and resources.
*/
static void FocuserAR0832_Close(NvOdmImagerHandle hImager)
{
    FocuserCtxt *pContext = NULL;

    NV_DEBUG_PRINTF(("%s\n", __func__));
    if (hImager && hImager->pFocuser && hImager->pFocuser->pPrivateContext)
    {
        pContext = (FocuserCtxt *)hImager->pFocuser->pPrivateContext;
#if (BUILD_FOR_AOS == 0)
        if (hImager->pPrivateContext)
        {
            AR0832_Stereo_Info *pStereo_Info = (AR0832_Stereo_Info *)hImager->pPrivateContext;
            pStereo_Info->drv_close(pStereo_Info);
        }
#endif
        NvOdmOsFree(pContext);
        hImager->pFocuser->pPrivateContext = NULL;
    }
}

/**
*Focuser_Open
*Initialize Focuser's private context.
*/
static NvBool FocuserAR0832_Open(NvOdmImagerHandle hImager)
{
    FocuserCtxt *ctxt = NULL;
#if (BUILD_FOR_AOS == 0)
    int drv_status;
#endif

    NV_DEBUG_PRINTF(("%s\n", __func__));

    if (!hImager || !hImager->pFocuser)
        return NV_FALSE;

    ctxt = NvOdmOsAlloc(sizeof(*ctxt));
    if (!ctxt)
        goto fail;

    hImager->pFocuser->pPrivateContext = ctxt;
    NvOdmOsMemset(ctxt, 0, sizeof(FocuserCtxt));

#if (BUILD_FOR_AOS == 0)
    ctxt->Config.settle_time = SETTLE_TIME;
    ctxt->Config.pos_actual_low = POS_ACTUAL_LOW;
    ctxt->Config.pos_actual_high = POS_ACTUAL_HIGH;
    ctxt->Config.pos_working_low = POS_ACTUAL_LOW;
    ctxt->Config.pos_working_high = POS_ACTUAL_HIGH;

    {
        AR0832_Stereo_Info *pStereo_Info = (AR0832_Stereo_Info *)hImager->pPrivateContext;

        NV_ASSERT(pStereo_Info);
        drv_status = pStereo_Info->drv_open(pStereo_Info);
        if (drv_status == AR0832_DRIVER_OPEN_NONE)
        {
            NvOsDebugPrintf("Can not open focuser device: %s\n", strerror(errno));
            goto fail;
        }
        ctxt->pStereo_Info = pStereo_Info;
    }
#endif

    ctxt->PowerLevel = NvOdmImagerPowerLevel_Off;
    ctxt->cmdTime = 0;
    ctxt->RequestedPosition = ctxt->Position = 0;
    ctxt->DelayedFPos = -1;
    ctxt->FocalLength = LENS_FOCAL_LENGTH;
    ctxt->FNumber = LENS_FNUMBER;
    ctxt->MaxAperture = LENS_MAX_APERTURE;

    NV_DEBUG_PRINTF(("Focuser_Open Successfully\n"));

    return NV_TRUE;

fail:
    NvOsDebugPrintf("Focuser_Open FAILED\n");
    FocuserAR0832_Close(hImager);
    return NV_FALSE;
}

/**
*Focuser_SetPowerLevel
*Set the focuser's power level.
*/
static NvBool FocuserAR0832_SetPowerLevel(NvOdmImagerHandle hImager,
                        NvOdmImagerPowerLevel PowerLevel)
{
    NvBool Status = NV_TRUE;
#if (BUILD_FOR_AOS == 0)
    FocuserCtxt *pContext = (FocuserCtxt *) hImager->pFocuser->pPrivateContext;
    NvS32 dPos;

    NV_DEBUG_PRINTF(("%s (%d)\n", __func__, PowerLevel));

    if (pContext->PowerLevel == PowerLevel)
        return NV_TRUE;

    switch(PowerLevel)
    {
        case NvOdmImagerPowerLevel_On:
            // Check for delayed focusing request:
            dPos = pContext->DelayedFPos;
            if (dPos >= 0)
            {
                Status = setPosition(pContext, dPos);
                pContext->DelayedFPos = -1;
            }
            break;
        case NvOdmImagerPowerLevel_Off:
            break;
        default:
            NvOsDebugPrintf("Focuser taking power level %d\n", PowerLevel);
            break;
    }

    pContext->PowerLevel = PowerLevel;
#endif
    return Status;
}

/**
*Focuser_GetCapabilities
*Get focuser's capabilities.
*/
static void FocuserAR0832_GetCapabilities(NvOdmImagerHandle hImager,
            NvOdmImagerCapabilities *pCapabilities)
{
    NV_DEBUG_PRINTF(("%s - stubbed\n", __func__));
}

/**
*Focuser_SetParameter
*Set focuser's parameter
*/
static NvBool FocuserAR0832_SetParameter(NvOdmImagerHandle hImager,
            NvOdmImagerParameter Param,
            NvS32 SizeOfValue, const void *pValue)
{
#if (BUILD_FOR_AOS == 0)
    NvBool Status = NV_FALSE;
    FocuserCtxt *pContext = (FocuserCtxt *) hImager->pFocuser->pPrivateContext;
    NvU32 i, j;

    switch (Param)
    {
        case NvOdmImagerParameter_FocuserLocus:
        {
            NvU32 Position = *((NvU32 *) pValue);
            NV_DEBUG_PRINTF(("Focuser_SetParameter NvOdmImagerParameter_FocuserLocus\n"));

            if (pContext->PowerLevel != NvOdmImagerPowerLevel_On)
            {
                pContext->DelayedFPos = (NvS32) Position;
                Status = NV_TRUE;
                break;
            }

            Status = setPosition(pContext, Position);
            break;
        }
        case NvOdmImagerParameter_FocuserCapabilities:
        {
#if (BUILD_FOR_AOS == 0)
            NvOdmImagerFocuserCapabilities *pCaps =
                (NvOdmImagerFocuserCapabilities *) pValue;

            //pContext->Config.actuator_range = FOCUSER_POSITIONS(pContext);
            pContext->Config.pos_working_low = pCaps->positionWorkingLow;
            pContext->Config.pos_working_high = pCaps->positionWorkingHigh;

            // The actual low and high are part of the ODM. Blocks-camera may not
            // know the first time, in which case, it will send down 0s.
            // Do not overwrite in that case.
            if(pCaps->positionActualLow != AF_INVALID_VALUE &&
               pCaps->positionActualHigh != AF_INVALID_VALUE)
            {
                pContext->Config.pos_actual_low = pCaps->positionActualLow;
                pContext->Config.pos_actual_high = pCaps->positionActualHigh;
            }
            pContext->Config.slew_rate = pCaps->slewRate;
            pContext->Config.circle_of_confusion = pCaps->circleOfConfusion;

            // Set the working settle time
            if(pCaps->settleTime != 0)
            {
                pContext->Config.settle_time = pCaps->settleTime;
            }

            NV_DEBUG_PRINTF(("%s: pCaps->afConfigSetSize %d",
                        __FUNCTION__, pCaps->afConfigSetSize));
            NV_DEBUG_PRINTF(("%s: posActualLow %d high %d positionWorkingLow %d"
                            " positionWorkingHigh %d settle_time %d slew_rate %d", __FUNCTION__,
                            pContext->Config.pos_actual_low,
                            pContext->Config.pos_actual_high,
                            pCaps->positionWorkingLow,
                            pCaps->positionWorkingHigh,
                            pContext->Config.settle_time,
                            pCaps->slewRate));

            for (i = 0; i < pCaps->afConfigSetSize; i++)
            {
                pContext->Config.focuser_set[i].posture =
                                        pCaps->afConfigSet[i].posture;
                pContext->Config.focuser_set[i].macro =
                                        pCaps->afConfigSet[i].macro;
                pContext->Config.focuser_set[i].hyper =
                                        pCaps->afConfigSet[i].hyper;
                pContext->Config.focuser_set[i].inf =
                                        pCaps->afConfigSet[i].inf;
                pContext->Config.focuser_set[i].hysteresis =
                                        pCaps->afConfigSet[i].hysteresis;
                pContext->Config.focuser_set[i].settle_time =
                                        pCaps->afConfigSet[i].settle_time;
                pContext->Config.focuser_set[i].macro_offset =
                                        pCaps->afConfigSet[i].macro_offset;
                pContext->Config.focuser_set[i].inf_offset =
                                        pCaps->afConfigSet[i].inf_offset;
                pContext->Config.focuser_set[i].num_dist_pairs =
                                        pCaps->afConfigSet[i].num_dist_pairs;

                NV_DEBUG_PRINTF(("i %d posture %d macro %d hyper %d inf %d "
                                 "hyst %d setttle_time %d macro_offset %d "
                                 "inf_offset %d num_pairs %d",
                    i, pCaps->afConfigSet[i].posture, pCaps->afConfigSet[i].macro,
                    pCaps->afConfigSet[i].hyper, pCaps->afConfigSet[i].inf,
                    pCaps->afConfigSet[i].hysteresis, pCaps->afConfigSet[i].settle_time,
                    pCaps->afConfigSet[i].macro_offset, pCaps->afConfigSet[i].inf_offset,
                    pCaps->afConfigSet[i].num_dist_pairs));

                for (j = 0; j < pCaps->afConfigSet[i].num_dist_pairs; j++)
                {
                    pContext->Config.focuser_set[i].dist_pair[j].fdn =
                                pCaps->afConfigSet[i].dist_pair[j].fdn;
                    pContext->Config.focuser_set[i].dist_pair[j].distance =
                                pCaps->afConfigSet[i].dist_pair[j].distance;

                    NV_DEBUG_PRINTF(("i %d j %d fdn %d distance %d", i, j,
                                pCaps->afConfigSet[i].dist_pair[j].fdn,
                                pCaps->afConfigSet[i].dist_pair[j].distance));
                }
            }

            if (pContext->pStereo_Info->drv_ioctl(
                    pContext->pStereo_Info, AR0832_FOCUSER_IOCTL_SET_CONFIG,
                    &pContext->Config) < 0)
            {
                NV_DEBUG_PRINTF(("%s: ioctl to set configrations failed - %s\n",
                                __FILE__, strerror(errno)));
                break;
            }

#endif

            Status = NV_TRUE;
            break;
        }
        default:
            NvOsDebugPrintf("%s - Unsupported param (%d)\n", __func__, Param);
    }
    return Status;
#else
    return NV_TRUE;
#endif
}

/**
*Focuser_GetParameter
*Get focuser's parameter
*/
static NvBool FocuserAR0832_GetParameter(NvOdmImagerHandle hImager,
            NvOdmImagerParameter Param,
            NvS32 SizeOfValue,
            void *pValue)
{
    NvF32 *pFValue = (NvF32*) pValue;
    NvU32 *pUValue = (NvU32*) pValue;
    FocuserCtxt *pContext = (FocuserCtxt *) hImager->pFocuser->pPrivateContext;

    switch(Param) {
        case NvOdmImagerParameter_FocuserLocus:
            // The last requested position has been settled?
            Focuser_UpdateSettledPosition(pContext);
            *pUValue = pContext->Position;
            NV_DEBUG_PRINTF(("%s: position = %d\n", __func__, *pUValue));
            break;

        case NvOdmImagerParameter_FocalLength:
            *pFValue = pContext->FocalLength;   // LENS_FOCAL_LENGTH
            NV_DEBUG_PRINTF(("%s: focal length = %f\n", __func__, *pFValue));
            break;

        case NvOdmImagerParameter_MaxAperture:
            *pFValue = pContext->MaxAperture;   // LENS_MAX_APERTURE
            NV_DEBUG_PRINTF(("%s: max aperture = %f\n", __func__, *pFValue));
            break;

        case NvOdmImagerParameter_FNumber:
            *pFValue = pContext->FNumber;       // LENS_FNUMBER
            NV_DEBUG_PRINTF(("%s: fnumber = %f\n", __func__, *pFValue));
            break;

        case NvOdmImagerParameter_FocuserCapabilities:
            if (SizeOfValue < (NvS32)sizeof(NvOdmImagerFocuserCapabilities))
                return NV_FALSE;
#if (BUILD_FOR_AOS == 0)
            {
                NvOdmImagerFocuserCapabilities *pCaps =
                    (NvOdmImagerFocuserCapabilities *) pValue;
                pCaps->version =  NVODMIMAGER_AF_FOCUSER_CODE_VERSION;
                pCaps->settleTime = pContext->Config.settle_time;

                pCaps->positionActualLow = pContext->Config.pos_actual_low;
                pCaps->positionActualHigh = pContext->Config.pos_actual_high;

                pCaps->positionWorkingLow = pContext->Config.pos_working_low;
                pCaps->positionWorkingHigh = pContext->Config.pos_working_high;
                pCaps->slewRate =  pContext->Config.slew_rate;
                pCaps->circleOfConfusion =  pContext->Config.circle_of_confusion;
                pCaps->afConfigSet[0].inf = pCaps->positionWorkingLow;
                pCaps->afConfigSet[0].macro = pCaps->positionWorkingHigh;
                pCaps->afConfigSet[0].hyper = pCaps->positionWorkingLow;
                NV_DEBUG_PRINTF(("%s: Actual low %d high %d, Working low %d "
                                 "high %d settleTime %d\n",
                                __func__, pCaps->positionActualLow,
                                pCaps->positionActualHigh,
                                pCaps->positionWorkingLow,
                                pCaps->positionWorkingHigh,
                                pCaps->settleTime));
            }
#endif
            break;
        default:
            NvOsDebugPrintf("%s: Unsupported param (%d)\n", __func__, Param);
            return NV_FALSE;
    }
    return NV_TRUE;
}

/**
*Focuser_GetHal.
*return the hal functions associated with focuser
*/
NvBool FocuserAR0832_GetHal(NvOdmImagerHandle hImager)
{
    if (!hImager || !hImager->pFocuser)
        return NV_FALSE;

    hImager->pFocuser->pfnOpen = FocuserAR0832_Open;
    hImager->pFocuser->pfnClose = FocuserAR0832_Close;
    hImager->pFocuser->pfnGetCapabilities = FocuserAR0832_GetCapabilities;
    hImager->pFocuser->pfnSetPowerLevel = FocuserAR0832_SetPowerLevel;
    hImager->pFocuser->pfnSetParameter = FocuserAR0832_SetParameter;
    hImager->pFocuser->pfnGetParameter = FocuserAR0832_GetParameter;

    return NV_TRUE;
}
