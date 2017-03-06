/*
 * Copyright (c) 2008-2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

/*
 * This file implements the pin-mux configuration tables for each I/O module.
 */

// THESE SETTINGS ARE PLATFORM-SPECIFIC (not SOC-specific).
// PLATFORM = T30 Cardhu

#include "nvodm_query_pinmux.h"
#include "nvassert.h"
#include "nvodm_disp.h"
#include "nvodm_services.h"

#define MINI_IOBOARD 1
#define MAXI_IOBOARD 2

#define IOBOARD MINI_IOBOARD

static const NvU32 s_MiniIo_Spi[] = {
    NvOdmSpiPinMap_Config2,
    0,
    0,
    NvOdmSpiPinMap_Config3,
    0,
    0,
};

static const NvU32 s_MiniIo_Ulpi[] = {
    0, //NvOdmUlpiPinMap_Config1,
};

static const NvU32 s_MiniIo_Pwm[] = {
    NvOdmPwmPinMap_Config3,
};

static const NvU32 s_MiniIo_Sdio[] = {
    NvOdmSdioPinMap_Config1,
    NvOdmSdioPinMap_Config1,
    NvOdmSdioPinMap_Config1,
    NvOdmSdioPinMap_Config1,
};

static const NvU32 s_MiniIo_Hdmi[] = {
    NvOdmHdmiPinMap_Config1,
};


static const NvU32 s_MiniIo_PciExpress[] = {
    NvOdmPciExpressPinMap_Config1,
};

static const NvU32 s_MiniIo_Display[] = {
    NvOdmDisplayPinMap_Config1,
    0,
};

static const NvU32 s_MiniIo_Dap[] = {
    NvOdmDapPinMap_Config1,
    NvOdmDapPinMap_Config1,
    0,
};

static const NvU32 s_MiniIo_I2c[] = {
    NvOdmI2cPinMap_Config1,
    NvOdmI2cPinMap_Config1,
    NvOdmI2cPinMap_Config2,
    NvOdmI2cPinMap_Config1,
    NvOdmI2cPinMap_Config1,
};

static const NvU32 s_MiniIo_Nand[] = {
    NvOdmNandPinMap_Config1,
};

static const NvU32 s_MiniIo_Uart[] = {
    //  FIXME: This is a guess
    NvOdmUartPinMap_Config1,
    NvOdmUartPinMap_Config1,
    0,
    NvOdmUartPinMap_Config1,
    0,
};
static const NvU32 s_MiniIo_VideoInput[] = {
    NvOdmVideoInputPinMap_Config2
};

//  HACKHACKHACK -- This module doesn't exist, but the display driver can't
//  cope with a single display.
static const NvU32 s_MiniIo_Tvo[] = {
    NvOdmTvoPinMap_Config1,
};

static const NvU32 s_MiniIo_Kbd[] = {
    NvOdmKbdPinMap_Config1,
};

static const NvU32 s_MiniIo_Crt[] = {
    NvOdmCrtPinMap_Config1,
};

static const NvU32 s_MiniIo_ExternalClock[] = {
    NvOdmExternalClockPinMap_Config1,
    NvOdmExternalClockPinMap_Config1,
    NvOdmExternalClockPinMap_Config1,
};

static const NvU32 s_MiniIo_OneWire[] = {
    NvOdmOneWirePinMap_Config1
};

static const NvU32 s_NvOdmClockLimit_Sdio[] = {
    26500,
    26500,
    26500,
    50000,
};

#define MODULE_ENTRY_BOARD(_b_,_x_) \
    case NvOdmIoModule_##_x_: \
        *pPinMuxConfigTable = s_##_b_##_##_x_; \
        *pCount = NV_ARRAY_SIZE(s_##_b_##_##_x_); \
        break;

#if IOBOARD == MINI_IOBOARD
#define MODULE_ENTRY(_x_) MODULE_ENTRY_BOARD(MiniIo,_x_)
#else
NV_CT_ASSERT(!"Not Supported");
#endif

void
NvOdmQueryPinMux(
    NvOdmIoModule IoModule,
    const NvU32 **pPinMuxConfigTable,
    NvU32 *pCount)
{
    NvOdmOsOsInfo OSInfo = {0};
    NvBool Status = NV_FALSE;

    switch (IoModule)
    {
        MODULE_ENTRY(I2c);
        MODULE_ENTRY(Display);
        MODULE_ENTRY(VideoInput);
        MODULE_ENTRY(Ulpi);
        MODULE_ENTRY(Nand);
        MODULE_ENTRY(Spi);
        MODULE_ENTRY(Tvo);
        MODULE_ENTRY(Crt);
        MODULE_ENTRY(Uart);
        MODULE_ENTRY(ExternalClock);
        MODULE_ENTRY(Dap);
        MODULE_ENTRY(Hdmi);
        MODULE_ENTRY(OneWire);
        MODULE_ENTRY(Kbd);
        MODULE_ENTRY(Sdio);
        MODULE_ENTRY(Pwm);

        case NvOdmIoModule_PciExpress:
        {
            Status = NvOdmOsGetOsInformation( &OSInfo );
            if (OSInfo.OsType ==NvOdmOsOs_Aos)
            {
                *pPinMuxConfigTable = s_MiniIo_PciExpress;
                *pCount = NV_ARRAY_SIZE(s_MiniIo_PciExpress);
            }
            else
            {
                *pPinMuxConfigTable = NULL;
                *pCount = 0;
            }
            break;
        }
    default:
        *pCount = 0;
        break;
    }
}

void
NvOdmQueryClockLimits(
    NvOdmIoModule IoModule,
    const NvU32 **pClockSpeedLimits,
    NvU32 *pCount)
{

    switch(IoModule)
    {
        case NvOdmIoModule_Sdio:
            *pClockSpeedLimits = s_NvOdmClockLimit_Sdio;
            *pCount = NV_ARRAY_SIZE(s_NvOdmClockLimit_Sdio);
            break;

        default:
        *pClockSpeedLimits = NULL;
        *pCount = 0;
        break;
    }
}

