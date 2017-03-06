/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

/**
 * @file
 * <b>NVIDIA APX ODM Kit:
 *         Implementation of the ODM Query API</b>
 *
 * @b Description: Implements the query functions for ODMs that may be
 *                 accessed at boot-time, runtime, or anywhere in between.
 */

#include "nvodm_query.h"
#include "nvodm_query_gpio.h"
#include "nvodm_query_memc.h"
#include "nvodm_query_discovery.h"
#ifndef SET_KERNEL_PINMUX
#include "nvodm_query_pins.h"
#include "nvodm_query_pins_ap20.h"
#else
#include "nvrm_pinmux.h"
#endif
#include "tegra_devkit_custopt.h"
#include "nvodm_keylist_reserved.h"
#include "nvrm_drf.h"
#include "nvbct.h"

#define BOARD_ID_WHISTLER_E1108 0x0B08
#define BOARD_ID_WHISTLER_E1109 0x0B09
#define BOARD_ID_WHISTLER_PMU_E1116 0x0B10
#define BOARD_ID_WHISTLER_MOTHERBOARD_E1120 0xB14
#define BOARD_ID_VOYAGER_MAINBOARD_E1215    0xC0F
#define BOARD_REV_ALL ((NvU8)0xFF)

#define NVODM_ENABLE_EMC_DVFS (0)

#if NV_DEBUG
static NvOdmDebugConsole
s_NvOdmQueryDebugConsoleSetting = NvOdmDebugConsole_Dcc;
#else
static NvOdmDebugConsole
s_NvOdmQueryDebugConsoleSetting = NvOdmDebugConsole_None;
#endif

// Function to auto-detect boards with external CPU power supply
NvBool NvOdmIsCpuExtSupply(void);

static const NvU8
s_NvOdmQueryDeviceNamePrefixValue[] = { 'T','e','g','r','a',0};

static const NvU8
s_NvOdmQueryManufacturerSetting[] = {'N','V','I','D','I','A',0};

static const NvU8
s_NvOdmQueryModelSetting[] = {'A','P','2','0',0};

static const NvU8
s_NvOdmQueryPlatformSetting[] = {'W','h','i','s','t','l','e','r',0};

static const NvU8
s_NvOdmQueryProjectNameSetting[] = {'O','D','M',' ','K','i','t',0};

static const NvOdmDownloadTransport
s_NvOdmQueryDownloadTransportSetting = NvOdmDownloadTransport_None;

static NvOdmQuerySdioInterfaceProperty s_NvOdmQuerySdioInterfaceProperty_Whistler[4] =
{
    { NV_FALSE, 10, NV_FALSE, 0x6, NvOdmQuerySdioSlotUsage_unused },
    {  NV_TRUE, 10,  NV_TRUE, 0xF, NvOdmQuerySdioSlotUsage_wlan   },
    { NV_FALSE, 10, NV_FALSE, 0x4, NvOdmQuerySdioSlotUsage_Media  },
    {  NV_TRUE, 10,  NV_TRUE, 0x6, NvOdmQuerySdioSlotUsage_Boot   },
};

static NvOdmQuerySdioInterfaceProperty s_NvOdmQuerySdioInterfaceProperty_Voyager[4] =
{
    { NV_FALSE, 10, NV_FALSE, 0x4, NvOdmQuerySdioSlotUsage_unused },
    {  NV_TRUE, 10,  NV_TRUE, 0x4, NvOdmQuerySdioSlotUsage_wlan   },
    { NV_FALSE, 10, NV_FALSE, 0x4, NvOdmQuerySdioSlotUsage_Media  },
    { NV_FALSE, 10, NV_FALSE, 0x4, NvOdmQuerySdioSlotUsage_Boot   },
};

static const NvOdmQueryOwrDeviceInfo s_NvOdmQueryOwrInfo = {
    NV_FALSE,
    0x1, /* Tsu */
    0xF, /* TRelease */
    0xF,  /* TRdv */
    0X3C, /* TLow0 */
    0x1, /* TLow1 */
    0x77, /* TSlot */

    0x78, /* TPdl */
    0x1E, /* TPdh */
    0x1DF, /* TRstl */
    0x1DF, /* TRsth */

    0x1E0, /* Tpp */
    0x5, /* Tfp */
    0x5, /* Trp */
    0x5, /* Tdv */
    0x5, /* Tpd */

    0x7, /* Read data sample clk */
    0x50, /* Presence sample clk */
    2,  /* Memory address size */
    0x80 /* Memory size*/
};

static const NvOdmSdramControllerConfigAdv s_NvOdmE1109EmcConfigTable[] =
{
    {
                  0x20,   /* Rev 2.0 */
                166500,   /* SDRAM frquency */
                  1000,   /* EMC core voltage */
                    46,   /* Number of EMC parameters below */
        {
            0x0000000A,   /* RC */
            0x00000016,   /* RFC */
            0x00000008,   /* RAS */
            0x00000003,   /* RP */
            0x00000004,   /* R2W */
            0x00000004,   /* W2R */
            0x00000002,   /* R2P */
            0x0000000F,   /* W2P */
            0x00000003,   /* RD_RCD */
            0x00000003,   /* WR_RCD */
            0x00000002,   /* RRD */
            0x00000001,   /* REXT */
            0x00000005,   /* WDV */
            0x00000006,   /* QUSE */
            0x00000005,   /* QRST */
            0x00000009,   /* QSAFE */
            0x0000000E,   /* RDV */
            0x000004DF,   /* REFRESH */
            0x00000000,   /* BURST_REFRESH_NUM */
            0x00000004,   /* PDEX2WR */
            0x00000004,   /* PDEX2RD */
            0x00000003,   /* PCHG2PDEN */
            0x00000003,   /* ACT2PDEN */
            0x00000001,   /* AR2PDEN */
            0x0000000B,   /* RW2PDEN */
            0x000000C8,   /* TXSR */
            0x00000003,   /* TCKE */
            0x00000008,   /* TFAW */
            0x00000004,   /* TRPAB */
            0x0000000B,   /* TCLKSTABLE */
            0x00000002,   /* TCLKSTOP */
            0x00000000,   /* TREFBW */
            0x00000000,   /* QUSE_EXTRA */
            0x00000002,   /* FBIO_CFG6 */
            0x00000000,   /* ODT_WRITE */
            0x00000000,   /* ODT_READ */
            0x00000083,   /* FBIO_CFG5 */
            0x00400006,   /* CFG_DIG_DLL */
            0x007FD010,   /* DLL_XFORM_DQS */
            0x00001010,   /* DLL_XFORM_QUSE */
            0x00000000,   /* ZCAL_REF_CNT */
            0x00000000,   /* ZCAL_WAIT_CNT */
            0x00000000,   /* AUTO_CAL_INTERVAL */
            0x00000000,   /* CFG_CLKTRIM_0 */
            0x00000000,   /* CFG_CLKTRIM_1 */
            0x00000000,   /* CFG_CLKTRIM_2 */
        }
    },
    {
                  0x20,   /* Rev 2.0 */
                333000,   /* SDRAM frquency */
                  1200,   /* EMC core voltage */
                    46,   /* Number of EMC parameters below */
        {
            0x00000014,   /* RC */
            0x0000002B,   /* RFC */
            0x0000000F,   /* RAS */
            0x00000005,   /* RP */
            0x00000004,   /* R2W */
            0x00000005,   /* W2R */
            0x00000003,   /* R2P */
            0x0000000F,   /* W2P */
            0x00000005,   /* RD_RCD */
            0x00000005,   /* WR_RCD */
            0x00000004,   /* RRD */
            0x00000001,   /* REXT */
            0x00000005,   /* WDV */
            0x00000006,   /* QUSE */
            0x00000005,   /* QRST */
            0x00000009,   /* QSAFE */
            0x0000000E,   /* RDV */
            0x000009FF,   /* REFRESH */
            0x00000000,   /* BURST_REFRESH_NUM */
            0x00000004,   /* PDEX2WR */
            0x00000004,   /* PDEX2RD */
            0x00000005,   /* PCHG2PDEN */
            0x00000005,   /* ACT2PDEN */
            0x00000001,   /* AR2PDEN */
            0x00000010,   /* RW2PDEN */
            0x000000C8,   /* TXSR */
            0x00000003,   /* TCKE */
            0x0000000F,   /* TFAW */
            0x00000006,   /* TRPAB */
            0x0000000B,   /* TCLKSTABLE */
            0x00000002,   /* TCLKSTOP */
            0x00000000,   /* TREFBW */
            0x00000000,   /* QUSE_EXTRA */
            0x00000002,   /* FBIO_CFG6 */
            0x00000000,   /* ODT_WRITE */
            0x00000000,   /* ODT_READ */
            0x00000083,   /* FBIO_CFG5 */
            0x002C0006,   /* CFG_DIG_DLL */
            0x007FD010,   /* DLL_XFORM_DQS */
            0x00001010,   /* DLL_XFORM_QUSE */
            0x00000000,   /* ZCAL_REF_CNT */
            0x00000000,   /* ZCAL_WAIT_CNT */
            0x00000000,   /* AUTO_CAL_INTERVAL */
            0x00000000,   /* CFG_CLKTRIM_0 */
            0x00000000,   /* CFG_CLKTRIM_1 */
            0x00000000,   /* CFG_CLKTRIM_2 */
        }
    }
};

static const NvOdmSdramControllerConfigAdv s_NvOdmE1108HynixEmcConfigTable[] =
{
    {
                  0x20,   /* Rev 2.0 */
                 18000,   /* SDRAM frquency */
                   950,   /* EMC core voltage */
                    46,   /* Number of EMC parameters below */
        {
            0x00000002,   /* RC */
            0x00000006,   /* RFC */
            0x00000003,   /* RAS */
            0x00000003,   /* RP */
            0x00000006,   /* R2W */
            0x00000004,   /* W2R */
            0x00000002,   /* R2P */
            0x0000000B,   /* W2P */
            0x00000003,   /* RD_RCD */
            0x00000003,   /* WR_RCD */
            0x00000002,   /* RRD */
            0x00000002,   /* REXT */
            0x00000003,   /* WDV */
            0x00000005,   /* QUSE */
            0x00000004,   /* QRST */
            0x00000008,   /* QSAFE */
            0x0000000C,   /* RDV */
            0x00000070,   /* REFRESH */
            0x00000000,   /* BURST_REFRESH_NUM */
            0x00000003,   /* PDEX2WR */
            0x00000003,   /* PDEX2RD */
            0x00000003,   /* PCHG2PDEN */
            0x00000003,   /* ACT2PDEN */
            0x00000001,   /* AR2PDEN */
            0x0000000B,   /* RW2PDEN */
            0x00000003,   /* TXSR */
            0x00000003,   /* TCKE */
            0x00000008,   /* TFAW */
            0x00000004,   /* TRPAB */
            0x00000008,   /* TCLKSTABLE */
            0x00000002,   /* TCLKSTOP */
            0x0000004B,   /* TREFBW */
            0x00000000,   /* QUSE_EXTRA */
            0x00000003,   /* FBIO_CFG6 */
            0x00000000,   /* ODT_WRITE */
            0x00000000,   /* ODT_READ */
            0x00000082,   /* FBIO_CFG5 */
            0x00780006,   /* CFG_DIG_DLL */
            0x00000010,   /* DLL_XFORM_DQS */
            0x00000008,   /* DLL_XFORM_QUSE */
            0x00000000,   /* ZCAL_REF_CNT */
            0x00000002,   /* ZCAL_WAIT_CNT */
            0x00000000,   /* AUTO_CAL_INTERVAL */
            0x00000000,   /* CFG_CLKTRIM_0 */
            0x00000000,   /* CFG_CLKTRIM_1 */
            0x00000000,   /* CFG_CLKTRIM_2 */
        }
    },
    {
                  0x20,   /* Rev 2.0 */
                 27000,   /* SDRAM frquency */
                   950,   /* EMC core voltage */
                    46,   /* Number of EMC parameters below */
        {
            0x00000002,   /* RC */
            0x00000006,   /* RFC */
            0x00000003,   /* RAS */
            0x00000003,   /* RP */
            0x00000006,   /* R2W */
            0x00000004,   /* W2R */
            0x00000002,   /* R2P */
            0x0000000B,   /* W2P */
            0x00000003,   /* RD_RCD */
            0x00000003,   /* WR_RCD */
            0x00000002,   /* RRD */
            0x00000002,   /* REXT */
            0x00000003,   /* WDV */
            0x00000005,   /* QUSE */
            0x00000004,   /* QRST */
            0x00000008,   /* QSAFE */
            0x0000000C,   /* RDV */
            0x000000A8,   /* REFRESH */
            0x00000000,   /* BURST_REFRESH_NUM */
            0x00000003,   /* PDEX2WR */
            0x00000003,   /* PDEX2RD */
            0x00000003,   /* PCHG2PDEN */
            0x00000003,   /* ACT2PDEN */
            0x00000001,   /* AR2PDEN */
            0x0000000B,   /* RW2PDEN */
            0x00000004,   /* TXSR */
            0x00000003,   /* TCKE */
            0x00000008,   /* TFAW */
            0x00000004,   /* TRPAB */
            0x00000008,   /* TCLKSTABLE */
            0x00000002,   /* TCLKSTOP */
            0x00000071,   /* TREFBW */
            0x00000000,   /* QUSE_EXTRA */
            0x00000003,   /* FBIO_CFG6 */
            0x00000000,   /* ODT_WRITE */
            0x00000000,   /* ODT_READ */
            0x00000082,   /* FBIO_CFG5 */
            0x00780006,   /* CFG_DIG_DLL */
            0x00000010,   /* DLL_XFORM_DQS */
            0x00000008,   /* DLL_XFORM_QUSE */
            0x00000000,   /* ZCAL_REF_CNT */
            0x00000003,   /* ZCAL_WAIT_CNT */
            0x00000000,   /* AUTO_CAL_INTERVAL */
            0x00000000,   /* CFG_CLKTRIM_0 */
            0x00000000,   /* CFG_CLKTRIM_1 */
            0x00000000,   /* CFG_CLKTRIM_2 */
        }
    },
    {
                  0x20,   /* Rev 2.0 */
                 54000,   /* SDRAM frquency */
                  1000,   /* EMC core voltage */
                    46,   /* Number of EMC parameters below */
        {
            0x00000004,   /* RC */
            0x00000008,   /* RFC */
            0x00000003,   /* RAS */
            0x00000003,   /* RP */
            0x00000006,   /* R2W */
            0x00000004,   /* W2R */
            0x00000002,   /* R2P */
            0x0000000B,   /* W2P */
            0x00000003,   /* RD_RCD */
            0x00000003,   /* WR_RCD */
            0x00000002,   /* RRD */
            0x00000002,   /* REXT */
            0x00000003,   /* WDV */
            0x00000006,   /* QUSE */
            0x00000004,   /* QRST */
            0x00000008,   /* QSAFE */
            0x0000000C,   /* RDV */
            0x0000017F,   /* REFRESH */
            0x00000000,   /* BURST_REFRESH_NUM */
            0x00000003,   /* PDEX2WR */
            0x00000003,   /* PDEX2RD */
            0x00000003,   /* PCHG2PDEN */
            0x00000003,   /* ACT2PDEN */
            0x00000001,   /* AR2PDEN */
            0x0000000B,   /* RW2PDEN */
            0x00000008,   /* TXSR */
            0x00000003,   /* TCKE */
            0x00000008,   /* TFAW */
            0x00000004,   /* TRPAB */
            0x00000008,   /* TCLKSTABLE */
            0x00000002,   /* TCLKSTOP */
            0x000000E1,   /* TREFBW */
            0x00000000,   /* QUSE_EXTRA */
            0x00000000,   /* FBIO_CFG6 */
            0x00000000,   /* ODT_WRITE */
            0x00000000,   /* ODT_READ */
            0x00000082,   /* FBIO_CFG5 */
            0x00780006,   /* CFG_DIG_DLL */
            0x00000010,   /* DLL_XFORM_DQS */
            0x00000008,   /* DLL_XFORM_QUSE */
            0x00000000,   /* ZCAL_REF_CNT */
            0x00000005,   /* ZCAL_WAIT_CNT */
            0x00000000,   /* AUTO_CAL_INTERVAL */
            0x00000000,   /* CFG_CLKTRIM_0 */
            0x00000000,   /* CFG_CLKTRIM_1 */
            0x00000000,   /* CFG_CLKTRIM_2 */
        }
    },
    {
                  0x20,   /* Rev 2.0 */
                108000,   /* SDRAM frquency */
                  1000,   /* EMC core voltage */
                    46,   /* Number of EMC parameters below */
        {
            0x00000007,   /* RC */
            0x0000000F,   /* RFC */
            0x00000005,   /* RAS */
            0x00000003,   /* RP */
            0x00000006,   /* R2W */
            0x00000004,   /* W2R */
            0x00000002,   /* R2P */
            0x0000000B,   /* W2P */
            0x00000003,   /* RD_RCD */
            0x00000003,   /* WR_RCD */
            0x00000002,   /* RRD */
            0x00000002,   /* REXT */
            0x00000003,   /* WDV */
            0x00000006,   /* QUSE */
            0x00000004,   /* QRST */
            0x00000008,   /* QSAFE */
            0x0000000C,   /* RDV */
            0x0000031F,   /* REFRESH */
            0x00000000,   /* BURST_REFRESH_NUM */
            0x00000003,   /* PDEX2WR */
            0x00000003,   /* PDEX2RD */
            0x00000003,   /* PCHG2PDEN */
            0x00000003,   /* ACT2PDEN */
            0x00000001,   /* AR2PDEN */
            0x0000000B,   /* RW2PDEN */
            0x00000010,   /* TXSR */
            0x00000003,   /* TCKE */
            0x00000008,   /* TFAW */
            0x00000004,   /* TRPAB */
            0x00000008,   /* TCLKSTABLE */
            0x00000002,   /* TCLKSTOP */
            0x000001C2,   /* TREFBW */
            0x00000000,   /* QUSE_EXTRA */
            0x00000000,   /* FBIO_CFG6 */
            0x00000000,   /* ODT_WRITE */
            0x00000000,   /* ODT_READ */
            0x00000082,   /* FBIO_CFG5 */
            0xD0780323,   /* CFG_DIG_DLL */
            0x007FD010,   /* DLL_XFORM_DQS */
            0x00000010,   /* DLL_XFORM_QUSE */
            0x00000000,   /* ZCAL_REF_CNT */
            0x0000000A,   /* ZCAL_WAIT_CNT */
            0x00000000,   /* AUTO_CAL_INTERVAL */
            0x00000000,   /* CFG_CLKTRIM_0 */
            0x00000000,   /* CFG_CLKTRIM_1 */
            0x00000000,   /* CFG_CLKTRIM_2 */
        }
    },
    {
                  0x20,   /* Rev 2.0 */
                150000,   /* SDRAM frquency */
                  1000,   /* EMC core voltage */
                    46,   /* Number of EMC parameters below */
        {
            0x00000009,   /* RC */
            0x00000014,   /* RFC */
            0x00000007,   /* RAS */
            0x00000003,   /* RP */
            0x00000006,   /* R2W */
            0x00000004,   /* W2R */
            0x00000002,   /* R2P */
            0x0000000B,   /* W2P */
            0x00000003,   /* RD_RCD */
            0x00000003,   /* WR_RCD */
            0x00000002,   /* RRD */
            0x00000002,   /* REXT */
            0x00000003,   /* WDV */
            0x00000006,   /* QUSE */
            0x00000004,   /* QRST */
            0x00000008,   /* QSAFE */
            0x0000000C,   /* RDV */
            0x0000045F,   /* REFRESH */
            0x00000000,   /* BURST_REFRESH_NUM */
            0x00000003,   /* PDEX2WR */
            0x00000003,   /* PDEX2RD */
            0x00000003,   /* PCHG2PDEN */
            0x00000003,   /* ACT2PDEN */
            0x00000001,   /* AR2PDEN */
            0x0000000B,   /* RW2PDEN */
            0x00000015,   /* TXSR */
            0x00000003,   /* TCKE */
            0x00000008,   /* TFAW */
            0x00000004,   /* TRPAB */
            0x00000008,   /* TCLKSTABLE */
            0x00000002,   /* TCLKSTOP */
            0x00000270,   /* TREFBW */
            0x00000000,   /* QUSE_EXTRA */
            0x00000001,   /* FBIO_CFG6 */
            0x00000000,   /* ODT_WRITE */
            0x00000000,   /* ODT_READ */
            0x00000082,   /* FBIO_CFG5 */
            0xD05E0323,   /* CFG_DIG_DLL */
            0x007FD010,   /* DLL_XFORM_DQS */
            0x00000010,   /* DLL_XFORM_QUSE */
            0x00000000,   /* ZCAL_REF_CNT */
            0x0000000E,   /* ZCAL_WAIT_CNT */
            0x00000000,   /* AUTO_CAL_INTERVAL */
            0x00000000,   /* CFG_CLKTRIM_0 */
            0x00000000,   /* CFG_CLKTRIM_1 */
            0x00000000,   /* CFG_CLKTRIM_2 */
        }
    },
    {
                  0x20,   /* Rev 2.0 */
                300000,   /* SDRAM frquency */
                  1100,   /* EMC core voltage */
                    46,   /* Number of EMC parameters below */
        {
            0x00000012,   /* RC */
            0x00000027,   /* RFC */
            0x0000000D,   /* RAS */
            0x00000006,   /* RP */
            0x00000007,   /* R2W */
            0x00000005,   /* W2R */
            0x00000003,   /* R2P */
            0x0000000B,   /* W2P */
            0x00000006,   /* RD_RCD */
            0x00000006,   /* WR_RCD */
            0x00000003,   /* RRD */
            0x00000003,   /* REXT */
            0x00000003,   /* WDV */
            0x00000006,   /* QUSE */
            0x00000004,   /* QRST */
            0x00000009,   /* QSAFE */
            0x0000000D,   /* RDV */
            0x000008FF,   /* REFRESH */
            0x00000000,   /* BURST_REFRESH_NUM */
            0x00000004,   /* PDEX2WR */
            0x00000004,   /* PDEX2RD */
            0x00000006,   /* PCHG2PDEN */
            0x00000006,   /* ACT2PDEN */
            0x00000001,   /* AR2PDEN */
            0x0000000F,   /* RW2PDEN */
            0x0000002A,   /* TXSR */
            0x00000003,   /* TCKE */
            0x0000000F,   /* TFAW */
            0x00000007,   /* TRPAB */
            0x00000007,   /* TCLKSTABLE */
            0x00000002,   /* TCLKSTOP */
            0x000004E0,   /* TREFBW */
            0x00000000,   /* QUSE_EXTRA */
            0x00000002,   /* FBIO_CFG6 */
            0x00000000,   /* ODT_WRITE */
            0x00000000,   /* ODT_READ */
            0x00000282,   /* FBIO_CFG5 */
            0xE03A0303,   /* CFG_DIG_DLL */
            0x007FD010,   /* DLL_XFORM_DQS */
            0x00000010,   /* DLL_XFORM_QUSE */
            0x00000000,   /* ZCAL_REF_CNT */
            0x0000001B,   /* ZCAL_WAIT_CNT */
            0x00000000,   /* AUTO_CAL_INTERVAL */
            0x00000000,   /* CFG_CLKTRIM_0 */
            0x00000000,   /* CFG_CLKTRIM_1 */
            0x00000000,   /* CFG_CLKTRIM_2 */
        }
    }
};


// Wake Events
static NvOdmWakeupPadInfo s_NvOdmWakeupPadInfo[] =
{
    {NV_FALSE,  0, NvOdmWakeupPadPolarity_Low},     // Wake Event  0 - ulpi_data4 (UART_RI)
    {NV_FALSE,  1, NvOdmWakeupPadPolarity_High},    // Wake Event  1 - gp3_pv[3] (BB_MOD, MODEM_RESET_OUT)
    {NV_FALSE,  2, NvOdmWakeupPadPolarity_High},    // Wake Event  2 - dvi_d3
    {NV_FALSE,  3, NvOdmWakeupPadPolarity_Low},     // Wake Event  3 - sdio3_dat1
    {NV_FALSE,  4, NvOdmWakeupPadPolarity_High},    // Wake Event  4 - hdmi_int (HDMI_HPD)
    {NV_FALSE,  5, NvOdmWakeupPadPolarity_High},    // Wake Event  5 - vgp[6] (VI_GP6, Flash_EN2)
    {NV_FALSE,  6, NvOdmWakeupPadPolarity_High},    // Wake Event  6 - gp3_pu[5] (GPS_ON_OFF, GPS_IRQ)
    {NV_FALSE,  7, NvOdmWakeupPadPolarity_AnyEdge}, // Wake Event  7 - gp3_pu[6] (GPS_INT, BT_IRQ)
    {NV_FALSE,  8, NvOdmWakeupPadPolarity_AnyEdge}, // Wake Event  8 - gmi_wp_n (Unused, goes to nand module)
    {NV_FALSE,  9, NvOdmWakeupPadPolarity_High},    // Wake Event  9 - gp3_ps[2] (KB_COL10)
    {NV_FALSE, 10, NvOdmWakeupPadPolarity_High},    // Wake Event 10 - gmi_ad21 (Accelerometer_TH/TAP)
    {NV_FALSE, 11, NvOdmWakeupPadPolarity_Low},     // Wake Event 11 - spi2_cs2 (PEN_INT, AUDIO-IRQ)
    {NV_FALSE, 12, NvOdmWakeupPadPolarity_Low},     // Wake Event 12 - spi2_cs1 (HEADSET_DET, not used)
    {NV_FALSE, 13, NvOdmWakeupPadPolarity_Low},     // Wake Event 13 - sdio1_dat1
    {NV_FALSE, 14, NvOdmWakeupPadPolarity_AnyEdge}, // Wake Event 14 - gp3_pv[6] (WLAN_INT)
    {NV_FALSE, 15, NvOdmWakeupPadPolarity_AnyEdge}, // Wake Event 15 - gmi_ad16  (SPI3_DOUT, DTV_SPI4_CS1)
    {NV_FALSE, 16, NvOdmWakeupPadPolarity_High},    // Wake Event 16 - rtc_irq
    {NV_FALSE, 17, NvOdmWakeupPadPolarity_High},    // Wake Event 17 - kbc_interrupt
    {NV_FALSE, 18, NvOdmWakeupPadPolarity_Low},     // Wake Event 18 - pwr_int (PMIC_INT)
    {NV_FALSE, 19, NvOdmWakeupPadPolarity_High},    // Wake Event 19 - usb_vbus_wakeup[0]
    {NV_FALSE, 20, NvOdmWakeupPadPolarity_High},    // Wake Event 20 - usb_vbus_wakeup[1]
    {NV_FALSE, 21, NvOdmWakeupPadPolarity_Low},     // Wake Event 21 - usb_iddig[0]
    {NV_FALSE, 22, NvOdmWakeupPadPolarity_Low},     // Wake Event 22 - usb_iddig[1]
    {NV_FALSE, 23, NvOdmWakeupPadPolarity_Low},     // Wake Event 23 - gmi_iordy (HSMMC_CLK)
    {NV_FALSE, 24, NvOdmWakeupPadPolarity_High},    // Wake Event 24 - gp3_pv[2] (BB_MOD, MODEM WAKEUP_AP15, SPI-SS)
    {NV_FALSE, 25, NvOdmWakeupPadPolarity_High},    // Wake Event 25 - gp3_ps[4] (KB_COL12)
    {NV_FALSE, 26, NvOdmWakeupPadPolarity_High},    // Wake Event 26 - gp3_ps[5] (KB_COL10)
    {NV_FALSE, 27, NvOdmWakeupPadPolarity_High},    // Wake Event 27 - gp3_ps[0] (KB_COL8)
    {NV_FALSE, 28, NvOdmWakeupPadPolarity_Low},     // Wake Event 28 - gp3_pq[6] (KB_ROW6)
    {NV_FALSE, 29, NvOdmWakeupPadPolarity_Low},     // Wake Event 29 - gp3_pq[7] (KB_ROW6)
    {NV_FALSE, 30, NvOdmWakeupPadPolarity_High}     // Wake Event 30 - dap1_dout (DAP1_DOUT)
};


/* --- Function Implementations ---*/

#ifdef SET_KERNEL_PINMUX
static const NvU32 s_NvOdmClockLimit_Sdio[] = {
    50000,
    32000,
    50000,
    50000,
};

NvError
NvOdmQueryModuleInterfaceCaps(
    NvOdmIoModule Module,
    NvU32 Instance,
    void *pCaps)
{
    switch (Module)
    {
        case NvOdmIoModule_Sdio:
            if ((Instance == 0) || (Instance == 1))
                ((NvRmModuleSdmmcInterfaceCaps *)pCaps)->MmcInterfaceWidth = 4;
            else if ((Instance == 2) || (Instance == 3))
                ((NvRmModuleSdmmcInterfaceCaps *)pCaps)->MmcInterfaceWidth = 8;
            else
                return NvError_NotSupported;
            return NvSuccess;

        case NvOdmIoModule_Pwm:
            ((NvRmModulePwmInterfaceCaps *)pCaps)->PwmOutputIdSupported = 0;
            return NvError_NotSupported;

        case NvOdmIoModule_Uart:
            if (Instance == 0)
                ((NvRmModuleUartInterfaceCaps *)pCaps)->NumberOfInterfaceLines = 8;
            else if ((Instance == 1) || (Instance == 2))
                ((NvRmModuleUartInterfaceCaps *)pCaps)->NumberOfInterfaceLines = 4;
            else if ((Instance == 3) || (Instance == 4))
                ((NvRmModuleUartInterfaceCaps *)pCaps)->NumberOfInterfaceLines = 0;
            else
                return NvError_NotSupported;
            return NvSuccess;

            default:
                break;
    }
    return NvError_NotSupported;
}

void
NvOdmQueryClockLimits(
    NvOdmIoModule IoModule,
    const NvU32 **pClockSpeedLimits,
    NvU32 *pCount)
{
    switch (IoModule)
    {
    case NvOdmIoModule_Hsmmc:
        *pCount = 0;
        break;

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
#endif

static NvBool
NvOdmIsBoardPresent(
    const NvOdmBoardInfo* pBoardList,
    NvU32 ListSize)
{
    NvU32 i;
    NvOdmBoardInfo BoardInfo;

    // Scan for presence of any board in the list
    // ID/SKU/FAB fields must match, revision may be masked
    if (pBoardList)
    {
        for (i=0; i < ListSize; i++)
        {
            if (NvOdmPeripheralGetBoardInfo(
                pBoardList[i].BoardID, &BoardInfo))
            {
                if ((pBoardList[i].Fab == BoardInfo.Fab) &&
                    (pBoardList[i].SKU == BoardInfo.SKU) &&
                    ((pBoardList[i].Revision == BOARD_REV_ALL) ||
                     (pBoardList[i].Revision == BoardInfo.Revision)) &&
                    ((pBoardList[i].MinorRevision == BOARD_REV_ALL) ||
                     (pBoardList[i].MinorRevision == BoardInfo.MinorRevision)))
                {
                    return NV_TRUE; // Board found
                }
            }
        }
    }
    return NV_FALSE;
}

#if NVODM_ENABLE_EMC_DVFS
static NvBool NvOdmIsE1108Hynix(void)
{
    // A list of Whistler E1108 processor boards with Hynix LPDDR2
    // charcterized by s_NvOdmE1108HynixEmcConfigTable (fill in
    // ID/SKU/FAB fields, revision fields are ignored)
    static const NvOdmBoardInfo s_WhistlerE1108Hynix[] =
    {
        // ID                      SKU     FAB   Rev            Minor Rev
        { BOARD_ID_WHISTLER_E1108, 0x0A14, 0x01, BOARD_REV_ALL, BOARD_REV_ALL},
        { BOARD_ID_WHISTLER_E1108, 0x0A1E, 0x01, BOARD_REV_ALL, BOARD_REV_ALL},
        { BOARD_ID_WHISTLER_E1108, 0x0A00, 0x02, BOARD_REV_ALL, BOARD_REV_ALL},
        { BOARD_ID_WHISTLER_E1108, 0x0A0A, 0x02, BOARD_REV_ALL, BOARD_REV_ALL}
    };
    return NvOdmIsBoardPresent(s_WhistlerE1108Hynix,
                               NV_ARRAY_SIZE(s_WhistlerE1108Hynix));
}
#endif

static NvBool NvOdmIsCpuRailPreserved(void)
{
    // A list of Whistler PMU boards that preserves CPU voltage across LP2/LP1
    static const NvOdmBoardInfo s_WhistlerCpuPreservedBoards[] =
    {
        // ID                          SKU     FAB   Rev            Minor Rev
        { BOARD_ID_WHISTLER_PMU_E1116, 0x0A0A, 0x01, BOARD_REV_ALL, BOARD_REV_ALL},
    };
    return NvOdmIsBoardPresent(s_WhistlerCpuPreservedBoards,
                               NV_ARRAY_SIZE(s_WhistlerCpuPreservedBoards));
}

NvBool NvOdmIsCpuExtSupply(void)
{
    // A list of Whistler processor boards that use external DCDC as CPU
    // power supply (fill in ID/SKU/FAB fields, revision fields are ignored)
    static const NvOdmBoardInfo s_WhistlerCpuExtSupplyBoards[] =
    {
        // ID                      SKU     FAB   Rev            Minor Rev
        { BOARD_ID_WHISTLER_E1108, 0x0A14, 0x01, BOARD_REV_ALL, BOARD_REV_ALL},
        { BOARD_ID_WHISTLER_E1108, 0x0A00, 0x02, BOARD_REV_ALL, BOARD_REV_ALL}
    };
    return NvOdmIsBoardPresent(s_WhistlerCpuExtSupplyBoards,
                               NV_ARRAY_SIZE(s_WhistlerCpuExtSupplyBoards));
}

static NvU32
GetBctKeyValue(void)
{
    NvOdmServicesKeyListHandle hKeyList = NULL;
    NvU32 BctCustOpt = 0;

    hKeyList = NvOdmServicesKeyListOpen();
    if (hKeyList)
    {
        BctCustOpt =
            NvOdmServicesGetKeyValue(hKeyList,
                                     NvOdmKeyListId_ReservedBctCustomerOption);
        NvOdmServicesKeyListClose(hKeyList);
    }

    return BctCustOpt;
}

NvOdmDebugConsole
NvOdmQueryDebugConsole(void)
{
    NvU32 CustOpt = GetBctKeyValue();
    switch (NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, CONSOLE, CustOpt))
    {
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_CONSOLE_DCC:
        return NvOdmDebugConsole_Dcc;
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_CONSOLE_NONE:
        return NvOdmDebugConsole_None;
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_CONSOLE_UART:
        return NvOdmDebugConsole_UartA +
            NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, CONSOLE_OPTION, CustOpt);
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_CONSOLE_AUTOMATION:
        return NvOdmDebugConsole_Automation;
    default:
        return s_NvOdmQueryDebugConsoleSetting;
    }
}


NvOdmDownloadTransport
NvOdmQueryDownloadTransport(void)
{
    NvU32 CustOpt = GetBctKeyValue();

    switch (NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, TRANSPORT, CustOpt))
    {
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_TRANSPORT_NONE:
        return NvOdmDownloadTransport_None;
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_TRANSPORT_USB:
        return NvOdmDownloadTransport_Usb;
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_TRANSPORT_ETHERNET:
        switch (NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, ETHERNET_OPTION, CustOpt))
        {
        case TEGRA_DEVKIT_BCT_CUSTOPT_0_ETHERNET_OPTION_SPI:
        case TEGRA_DEVKIT_BCT_CUSTOPT_0_ETHERNET_OPTION_DEFAULT:
        default:
            return NvOdmDownloadTransport_SpiEthernet;
        }
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_TRANSPORT_UART:
        switch (NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, UART_OPTION, CustOpt))
        {
        case TEGRA_DEVKIT_BCT_CUSTOPT_0_UART_OPTION_B:
            return NvOdmDownloadTransport_UartB;
        case TEGRA_DEVKIT_BCT_CUSTOPT_0_UART_OPTION_C:
            return NvOdmDownloadTransport_UartC;
        case TEGRA_DEVKIT_BCT_CUSTOPT_0_UART_OPTION_DEFAULT:
        case TEGRA_DEVKIT_BCT_CUSTOPT_0_UART_OPTION_A:
        default:
            return NvOdmDownloadTransport_UartA;
        }
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_TRANSPORT_DEFAULT:
    default:
        return s_NvOdmQueryDownloadTransportSetting;
    }
}

const NvU8*
NvOdmQueryDeviceNamePrefix(void)
{
    return s_NvOdmQueryDeviceNamePrefixValue;
}

static const NvOdmQuerySpdifInterfaceProperty s_NvOdmQuerySpdifInterfacePropertySetting =
{
    NvOdmQuerySpdifDataCaptureControl_FromLeft
};

const NvOdmQuerySpiDeviceInfo *

NvOdmQuerySpiGetDeviceInfo(
    NvOdmIoModule OdmIoModule,
    NvU32 ControllerId,
    NvU32 ChipSelect)
{
    static const NvOdmQuerySpiDeviceInfo s_Spi1Cs0Info_EmpRil =
        {NvOdmQuerySpiSignalMode_0, NV_TRUE, NV_TRUE};

    static const NvOdmQuerySpiDeviceInfo s_Spi1Cs0Info_IfxRil =
        {NvOdmQuerySpiSignalMode_1, NV_TRUE, NV_FALSE};

    static const NvOdmQuerySpiDeviceInfo s_Spi1Cs0Info =
        {NvOdmQuerySpiSignalMode_0, NV_TRUE, NV_FALSE};

    static const NvOdmQuerySpiDeviceInfo s_Spi2Cs1Info =
        {NvOdmQuerySpiSignalMode_0, NV_TRUE, NV_FALSE};

    static const NvOdmQuerySpiDeviceInfo s_Spi3Cs1Info =
        {NvOdmQuerySpiSignalMode_0, NV_TRUE, NV_FALSE};

    NvU32 CustOpt = GetBctKeyValue();
    switch (NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, RIL, CustOpt))
    {
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_RIL_EMP_RAINBOW:
        if ((OdmIoModule == NvOdmIoModule_Spi) &&
            (ControllerId == 0 ) && (ChipSelect == 0))
            return &s_Spi1Cs0Info_EmpRil;
        break;
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_RIL_IFX:
        if ((OdmIoModule == NvOdmIoModule_Spi) &&
            (ControllerId == 0 ) && (ChipSelect == 0))
            return &s_Spi1Cs0Info_IfxRil;
        break;
    }

    if ((OdmIoModule == NvOdmIoModule_Spi) &&
        (ControllerId == 1 || ControllerId == 0) && (ChipSelect == 0))
        return &s_Spi1Cs0Info;

    if ((OdmIoModule == NvOdmIoModule_Spi) &&
        (ControllerId == 1 ) && ((ChipSelect == 1)|| (ChipSelect == 3)))
        return &s_Spi2Cs1Info;

    if ((OdmIoModule == NvOdmIoModule_Spi) &&
        (ControllerId == 2 ) && (ChipSelect == 1))
        return &s_Spi3Cs1Info;

    if ((OdmIoModule == NvOdmIoModule_Spi) &&
        (ControllerId == 2 ) && (ChipSelect == 2))
        return &s_Spi3Cs1Info;

    return NULL;
}


const NvOdmQuerySpiIdleSignalState *
NvOdmQuerySpiGetIdleSignalState(
    NvOdmIoModule OdmIoModule,
    NvU32 ControllerId)
{
    return NULL;
}

const NvOdmQueryI2sInterfaceProperty *
NvOdmQueryI2sGetInterfaceProperty(
    NvU32 I2sInstanceId)
{
    static const NvOdmQueryI2sInterfaceProperty s_Property =
    {
        NvOdmQueryI2sMode_Slave,
        NvOdmQueryI2sLRLineControl_LeftOnLow,
        NvOdmQueryI2sDataCommFormat_I2S,
        NV_FALSE,
        0
    };

    if ((!I2sInstanceId) || (I2sInstanceId == 1))
        return &s_Property;

    return NULL;
}

const NvOdmQueryDapPortProperty *
NvOdmQueryDapPortGetProperty(
    NvU32 DapPortId)
{
    static const NvOdmQueryDapPortProperty s_Property[] =
    {
        { NvOdmDapPort_None, NvOdmDapPort_None, { 0, 0, 0, 0 } },
        // I2S1 (DAC1) <-> DAP1 <-> HIFICODEC
        { NvOdmDapPort_I2s1, NvOdmDapPort_HifiCodecType,
          { 2, 16, 44100, NvOdmQueryI2sDataCommFormat_I2S } }, // Dap1
          // I2S2 (DAC2) <-> DAP2 <-> VOICECODEC
        {NvOdmDapPort_I2s2, NvOdmDapPort_VoiceCodecType,
          {2, 16, 8000, NvOdmQueryI2sDataCommFormat_I2S } },   // Dap2
        {NvOdmDapPort_None, NvOdmDapPort_None , {0, 0, 0, 0} }, // Dap3
        // I2S2 (DAC2) <-> DAP4 <-> BLUETOOTH
        {NvOdmDapPort_I2s2, NvOdmDapPort_BlueTooth,
          {2, 16, 8000, NvOdmQueryI2sDataCommFormat_I2S } },   // Dap4
    };
    static const NvOdmQueryDapPortProperty s_Property_Ril_Emp_Rainbow[] =
    {
        { NvOdmDapPort_None, NvOdmDapPort_None, { 0, 0, 0, 0 } },
        // I2S1 (DAC1) <-> DAP1 <-> HIFICODEC
        { NvOdmDapPort_I2s1, NvOdmDapPort_HifiCodecType,
          { 2, 16, 44100, NvOdmQueryI2sDataCommFormat_I2S } }, // Dap1
        // I2S2 (DAC2) <-> DAP2 <-> VOICECODEC
        {NvOdmDapPort_I2s2, NvOdmDapPort_VoiceCodecType,
          {2, 16, 8000, NvOdmQueryI2sDataCommFormat_I2S } },   // Dap2
        // I2S2 (DAC2) <-> DAP3 <-> BASEBAND
        {NvOdmDapPort_I2s2, NvOdmDapPort_BaseBand,
          {2, 16, 8000, NvOdmQueryI2sDataCommFormat_I2S } },   // Dap3
        // I2S2 (DAC2) <-> DAP4 <-> BLUETOOTH
        {NvOdmDapPort_I2s2, NvOdmDapPort_BlueTooth,
          {2, 16, 8000, NvOdmQueryI2sDataCommFormat_I2S } },   // Dap4
    };

    NvU32 CustOpt = GetBctKeyValue();
    switch (NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, RIL, CustOpt))
    {
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_RIL_EMP_RAINBOW:
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_RIL_EMP_RAINBOW_ULPI:
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_RIL_IFX:
        if (DapPortId && DapPortId<NV_ARRAY_SIZE(s_Property_Ril_Emp_Rainbow))
            return &s_Property_Ril_Emp_Rainbow[DapPortId];
        break;
    default:
        if (DapPortId && DapPortId<NV_ARRAY_SIZE(s_Property))
            return &s_Property[DapPortId];
        break;
    }
    return NULL;
}

const NvOdmQueryDapPortConnection*
NvOdmQueryDapPortGetConnectionTable(
    NvU32 ConnectionIndex)
{
    static const NvOdmQueryDapPortConnection s_Property_Ril_Emp_Rainbow[] =
    {
        { NvOdmDapConnectionIndex_Music_Path,
          2, { {NvOdmDapPort_I2s1, NvOdmDapPort_Dap1, NV_FALSE},
               {NvOdmDapPort_Dap1, NvOdmDapPort_I2s1, NV_TRUE} }},

        { NvOdmDapConnectionIndex_BlueTooth_Codec,
          4, { {NvOdmDapPort_Dap4, NvOdmDapPort_I2s2, NV_TRUE},
             {NvOdmDapPort_I2s2, NvOdmDapPort_Dap4, NV_FALSE},
             {NvOdmDapPort_I2s1, NvOdmDapPort_Dap1, NV_FALSE},
             {NvOdmDapPort_Dap1, NvOdmDapPort_I2s1, NV_TRUE} }},

        // Voicecall without Bluetooth
        { NvOdmDapConnectionIndex_VoiceCall_NoBlueTooth,
          4, { {NvOdmDapPort_Dap3, NvOdmDapPort_Dap2, NV_FALSE},
               {NvOdmDapPort_Dap2, NvOdmDapPort_Dap3, NV_TRUE},
               {NvOdmDapPort_I2s1, NvOdmDapPort_Dap1, NV_FALSE},
               {NvOdmDapPort_Dap1, NvOdmDapPort_I2s1, NV_TRUE} }},

        // Voicecall being recorded without Bluetooth
        { NvOdmDapConnectionIndex_VoiceCall_NoBlueTooth_Record,
          4, { {NvOdmDapPort_I2s1, NvOdmDapPort_Dap1, NV_FALSE},
               {NvOdmDapPort_Dap1, NvOdmDapPort_I2s1, NV_TRUE},
               {NvOdmDapPort_Dap3, NvOdmDapPort_I2s2, NV_FALSE},
               {NvOdmDapPort_I2s2, NvOdmDapPort_Dap3, NV_TRUE} }},

        // Voicecall with Bluetooth
        { NvOdmDapConnectionIndex_VoiceCall_WithBlueTooth,
          4, { {NvOdmDapPort_Dap4, NvOdmDapPort_Dap3, NV_TRUE},
               {NvOdmDapPort_Dap3, NvOdmDapPort_Dap4, NV_FALSE},
               {NvOdmDapPort_I2s1, NvOdmDapPort_Dap1, NV_FALSE},
               {NvOdmDapPort_Dap1, NvOdmDapPort_I2s1, NV_TRUE} }},

        // Voicecall being recorded with Bluetooth
        { NvOdmDapConnectionIndex_VoiceCall_WithBlueTooth_Record,
          4, { {NvOdmDapPort_I2s1, NvOdmDapPort_Dap4, NV_FALSE},
               {NvOdmDapPort_Dap4, NvOdmDapPort_I2s1, NV_TRUE},
               {NvOdmDapPort_Dap3, NvOdmDapPort_I2s2, NV_FALSE},
               {NvOdmDapPort_I2s2, NvOdmDapPort_Dap3, NV_TRUE} }},
    };

    NvU32 TableIndex = 0;
    for( TableIndex = 0; TableIndex < NV_ARRAY_SIZE(s_Property_Ril_Emp_Rainbow); TableIndex++)
    {
        if (s_Property_Ril_Emp_Rainbow[TableIndex].UseIndex == ConnectionIndex)
            return &s_Property_Ril_Emp_Rainbow[TableIndex];
    }
    return NULL;
}

const NvOdmQuerySpdifInterfaceProperty *
NvOdmQuerySpdifGetInterfaceProperty(
    NvU32 SpdifInstanceId)
{
    if (SpdifInstanceId == 0)
        return &s_NvOdmQuerySpdifInterfacePropertySetting;

    return NULL;
}

const NvOdmQueryAc97InterfaceProperty *
NvOdmQueryAc97GetInterfaceProperty(
    NvU32 Ac97InstanceId)
{
    return NULL;
}

const NvOdmQueryI2sACodecInterfaceProp *
NvOdmQueryGetI2sACodecInterfaceProperty(
    NvU32 AudioCodecId)
{
    static const NvOdmQueryI2sACodecInterfaceProp s_Property =
    {
        NV_TRUE,
        0,
        0x34,
        NV_FALSE,
        NvOdmQueryI2sLRLineControl_LeftOnLow,
        NvOdmQueryI2sDataCommFormat_I2S
    };
    if (!AudioCodecId)
        return &s_Property;
    return NULL;
}

NvBool NvOdmQueryAsynchMemConfig(
    NvU32 ChipSelect,
    NvOdmAsynchMemConfig *pMemConfig)
{
    return NV_FALSE;
}

const void*
NvOdmQuerySdramControllerConfigGet(NvU32 *pEntries, NvU32 *pRevision)
{
#if NVODM_ENABLE_EMC_DVFS
    NvOdmBoardInfo BoardInfo;

    if (NvOdmPeripheralGetBoardInfo(BOARD_ID_WHISTLER_E1109, &BoardInfo))
    {
        if (pRevision)
            *pRevision = s_NvOdmE1109EmcConfigTable[0].Revision;
        if (pEntries)
            *pEntries = NV_ARRAY_SIZE(s_NvOdmE1109EmcConfigTable);
        return (const void*)s_NvOdmE1109EmcConfigTable;
    }
    else if (NvOdmIsE1108Hynix())
    {
        if (pRevision)
            *pRevision = s_NvOdmE1108HynixEmcConfigTable[0].Revision;
        if (pEntries)
            *pEntries = NV_ARRAY_SIZE(s_NvOdmE1108HynixEmcConfigTable);
        return (const void*)s_NvOdmE1108HynixEmcConfigTable;
    }
#endif
    if (pEntries)
        *pEntries = 0;
    return NULL;
}

NvOdmQueryOscillator NvOdmQueryGetOscillatorSource(void)
{
    return NvOdmQueryOscillator_Xtal;
}

NvU32 NvOdmQueryGetOscillatorDriveStrength(void)
{
    return 0x07;
}

const NvOdmWakeupPadInfo *NvOdmQueryGetWakeupPadTable(NvU32 *pSize)
{
    NvU32 CustOpt;
    if (pSize)
    	*pSize = NV_ARRAY_SIZE(s_NvOdmWakeupPadInfo);


    CustOpt = GetBctKeyValue();
    switch (NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, RIL, CustOpt))
    {
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_RIL_EMP_RAINBOW:
        s_NvOdmWakeupPadInfo[24].enable = NV_TRUE;
        s_NvOdmWakeupPadInfo[24].Polarity = NvOdmWakeupPadPolarity_Low;
        break;
    case TEGRA_DEVKIT_BCT_CUSTOPT_0_RIL_IFX:
        s_NvOdmWakeupPadInfo[13].enable = NV_TRUE;
        s_NvOdmWakeupPadInfo[13].Polarity = NvOdmWakeupPadPolarity_High;
        break;
    default:
        break;
    }

    return (const NvOdmWakeupPadInfo *) s_NvOdmWakeupPadInfo;
}

const NvU8* NvOdmQueryManufacturer(void)
{
    return s_NvOdmQueryManufacturerSetting;
}

const NvU8* NvOdmQueryModel(void)
{
    return s_NvOdmQueryModelSetting;
}

const NvU8* NvOdmQueryPlatform(void)
{
    return s_NvOdmQueryPlatformSetting;
}

const NvU8* NvOdmQueryProjectName(void)
{
    return s_NvOdmQueryProjectNameSetting;
}

#ifndef SET_KERNEL_PINMUX

#define EXT 0     // external pull-up/down resistor
#define INT_PU 1  // internal pull-up
#define INT_PD 2  // internal pull-down

#define HIGHSPEED 1
#define SCHMITT 1
#define VREF    1
#define OHM_50 3
#define OHM_100 2
#define OHM_200 1
#define OHM_400 0

 // Pin attributes
 static const NvOdmPinAttrib s_pin_config_attributes[] = {

    // Pull ups for the kbc pins
    { NvOdmPinRegister_Ap20_PullUpDown_B,
     NVODM_QUERY_PIN_AP20_PULLUPDOWN_B(0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0) },

    { NvOdmPinRegister_Ap20_PullUpDown_D,
     NVODM_QUERY_PIN_AP20_PULLUPDOWN_D(0x2, 0x2, 0x0, 0x2, 0x2, 0x2, 0x1, 0x1, 0x1, 0x1, 0x2, 0x0, 0x1, 0x0, 0x2, 0x2) },

    // Pull ups for the kbc pins
    { NvOdmPinRegister_Ap20_PullUpDown_E,
     NVODM_QUERY_PIN_AP20_PULLUPDOWN_E(0x2, 0x2, 0x0, 0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2) },

    // Set pad control for the sdio2 - - AOCFG1 and AOCFG2 pad control register
    { NvOdmPinRegister_Ap20_PadCtrl_AOCFG1PADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_50, 31, 31, 3, 3) },

    { NvOdmPinRegister_Ap20_PadCtrl_AOCFG2PADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_50, 31, 31, 3, 3) },

    // Set pad control for the sdio1 - SDIO1 pad control register
    { NvOdmPinRegister_Ap20_PadCtrl_SDIO1CFGPADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_50, 31, 31, 3, 3) },

    // Set pad control for the sdio3 - SDIO2 and SDIO3 pad control register
    { NvOdmPinRegister_Ap20_PadCtrl_SDIO2CFGPADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_50, 31, 31, 3, 3) },

    { NvOdmPinRegister_Ap20_PadCtrl_SDIO3CFGPADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_50, 31, 31, 3, 3) },

    // Set pad control for the sdio4- ATCCFG1 and ATCCFG2 pad control register
    { NvOdmPinRegister_Ap20_PadCtrl_ATCFG1PADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_50, 31, 31, 3, 3) },

    { NvOdmPinRegister_Ap20_PadCtrl_ATCFG2PADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_50, 31, 31, 3, 3) },

    // Set pad control for I2C1 pins
    { NvOdmPinRegister_Ap20_PadCtrl_DBGCFGPADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_50, 31, 31, 3, 3) },

    { NvOdmPinRegister_Ap20_PadCtrl_VICFG1PADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_50, 31, 31, 3, 3) },

    { NvOdmPinRegister_Ap20_PadCtrl_VICFG2PADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_50, 31, 31, 3, 3) },

    // Set pad control for I2C2 (DDC) pins
    { NvOdmPinRegister_Ap20_PadCtrl_DDCCFGPADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_50, 31, 31, 3, 3) },

    // Set pad control for Dap pins 1 - 4
    { NvOdmPinRegister_Ap20_PadCtrl_DAP1CFGPADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_400, 0, 0, 0, 0) },

    { NvOdmPinRegister_Ap20_PadCtrl_DAP2CFGPADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_400, 0, 0, 0, 0) },

    { NvOdmPinRegister_Ap20_PadCtrl_DAP3CFGPADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_400, 0, 0, 0, 0) },

    { NvOdmPinRegister_Ap20_PadCtrl_DAP3CFGPADCTRL,
      NVODM_QUERY_PIN_AP20_PADCTRL_AOCFG1PADCTRL(!HIGHSPEED, SCHMITT, OHM_400, 0, 0, 0, 0) },
 };

NvU32
NvOdmQueryPinAttributes(const NvOdmPinAttrib** pPinAttributes)
{
    *pPinAttributes = &s_pin_config_attributes[0];
    return NV_ARRAY_SIZE(s_pin_config_attributes);
}
#endif

NvBool NvOdmPlatformConfigure(int Config)
{
    return NV_TRUE;
}

NvBool NvOdmQueryGetPmuProperty(NvOdmPmuProperty* pPmuProperty)
{
    pPmuProperty->IrqConnected = NV_FALSE;
    pPmuProperty->PowerGoodCount = 0x7E;
    pPmuProperty->IrqPolarity = NvOdmInterruptPolarity_Low;

    // Not there yet, add it later ...
    //pPmuProperty->CpuPowerReqPolarity = ;

    pPmuProperty->CorePowerReqPolarity = NvOdmCorePowerReqPolarity_High;
    pPmuProperty->SysClockReqPolarity = NvOdmSysClockReqPolarity_High;
    pPmuProperty->CombinedPowerReq = NV_TRUE;
    pPmuProperty->CpuPowerGoodUs = 2000;
    pPmuProperty->AccuracyPercent = 3;

    if (NvOdmIsCpuExtSupply() ||
        NvOdmIsCpuRailPreserved())
        pPmuProperty->VCpuOTPOnWakeup = NV_FALSE;
    else
        pPmuProperty->VCpuOTPOnWakeup = NV_TRUE;

    return NV_TRUE;
}

/**
 * Gets the lowest soc power state supported by the hardware
 *
 * @returns information about the SocPowerState
 */
const NvOdmSocPowerStateInfo* NvOdmQueryLowestSocPowerState(void)
{
    NvOdmServicesKeyListHandle  hKeyList;
    NvU32                       LPStateSelection = 0;
    static                      NvOdmSocPowerStateInfo  PowerStateInfo;
    const static                NvOdmSocPowerStateInfo* pPowerStateInfo = NULL;

    if (pPowerStateInfo == NULL)
    {
        hKeyList = NvOdmServicesKeyListOpen();
        if (hKeyList)
        {
            LPStateSelection = NvOdmServicesGetKeyValue(hKeyList,
                                                NvOdmKeyListId_ReservedBctCustomerOption);
            NvOdmServicesKeyListClose(hKeyList);
            LPStateSelection = NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, LPSTATE, LPStateSelection);
        }

        // Lowest power state controlled by the flashed custom option.
        PowerStateInfo.LowestPowerState = ((LPStateSelection == TEGRA_DEVKIT_BCT_CUSTOPT_0_LPSTATE_LP1)?
                                            NvOdmSocPowerState_Suspend : NvOdmSocPowerState_DeepSleep);
        // Idle threshold (Msecs) for the lowest power state.
        PowerStateInfo.IdleThreshold = 525;
        pPowerStateInfo = (const NvOdmSocPowerStateInfo*) &PowerStateInfo;
    }
    return (pPowerStateInfo);
}

const NvOdmUsbProperty*
NvOdmQueryGetUsbProperty(NvOdmIoModule OdmIoModule,
                         NvU32 Instance)
{
    static const NvOdmUsbProperty Usb1Property =
    {
        NvOdmUsbInterfaceType_Utmi,
        (NvOdmUsbChargerType_SE0 | NvOdmUsbChargerType_SE1 | NvOdmUsbChargerType_SK),
        20,
        NV_TRUE,
        NvOdmUsbModeType_Device,
        NvOdmUsbIdPinType_CableId,
        NvOdmUsbConnectorsMuxType_None,
        NV_FALSE
    };

    static const NvOdmUsbProperty Usb2Property =
    {
        NvOdmUsbInterfaceType_UlpiExternalPhy,
        NvOdmUsbChargerType_UsbHost,
        20,
        NV_TRUE,
        NvOdmUsbModeType_None,
        NvOdmUsbIdPinType_None,
        NvOdmUsbConnectorsMuxType_None,
        NV_FALSE
    };

    static const NvOdmUsbProperty Usb2NullPhyProperty =
    {
        NvOdmUsbInterfaceType_UlpiNullPhy,
        NvOdmUsbChargerType_UsbHost,
        20,
        NV_TRUE,
        NvOdmUsbModeType_Host,
        NvOdmUsbIdPinType_None,
        NvOdmUsbConnectorsMuxType_None,
        NV_FALSE,
        {10, 1, 1, 1}
    };

    static const NvOdmUsbProperty Usb3Property =
    {
        NvOdmUsbInterfaceType_Utmi,
        NvOdmUsbChargerType_UsbHost,
        20,
        NV_TRUE,
        NvOdmUsbModeType_Host,
        NvOdmUsbIdPinType_CableId,
        NvOdmUsbConnectorsMuxType_None,
        NV_FALSE
    };

    if (OdmIoModule == NvOdmIoModule_Usb && Instance == 0)
        return &(Usb1Property);

    if (OdmIoModule == NvOdmIoModule_Usb && Instance == 1)
    {
        NvU32 CustOpt = GetBctKeyValue();

        if (NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, RIL, CustOpt) ==
            TEGRA_DEVKIT_BCT_CUSTOPT_0_RIL_EMP_RAINBOW_ULPI)
            return &(Usb2NullPhyProperty);
        else
            return &(Usb2Property);
    }

    if (OdmIoModule == NvOdmIoModule_Usb && Instance == 2)
        return &(Usb3Property);

    return (const NvOdmUsbProperty *)NULL;
}

const NvOdmQuerySdioInterfaceProperty* NvOdmQueryGetSdioInterfaceProperty(NvU32 Instance)
{
    NvU32 CustomerOption = 0;
    NvU32 Personality = 0;
    NvOdmServicesKeyListHandle hKeyList;
    static NvBool s_IsVoyagerBoard = NV_FALSE;
    NvOdmBoardInfo BoardInfo;
    static NvBool s_IsBoardInfoDone = NV_FALSE;

    // Detect whether the board is voyager or not
    if (!s_IsBoardInfoDone)
    {
        s_IsVoyagerBoard = NvOdmPeripheralGetBoardInfo(
                            BOARD_ID_VOYAGER_MAINBOARD_E1215, &BoardInfo);
        s_IsBoardInfoDone = NV_TRUE;
    }

    hKeyList = NvOdmServicesKeyListOpen();
    if (hKeyList)
    {
        CustomerOption =
            NvOdmServicesGetKeyValue(hKeyList,
                                     NvOdmKeyListId_ReservedBctCustomerOption);
        NvOdmServicesKeyListClose(hKeyList);
        Personality =
            NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, PERSONALITY, CustomerOption);
    }

    if (!Personality)
        Personality = TEGRA_DEVKIT_DEFAULT_PERSONALITY;

    if (s_IsVoyagerBoard)
        return &s_NvOdmQuerySdioInterfaceProperty_Voyager[Instance];

    if (Personality == TEGRA_DEVKIT_BCT_CUSTOPT_0_PERSONALITY_75 ||
        Personality == TEGRA_DEVKIT_BCT_CUSTOPT_0_PERSONALITY_15 ||
        Personality == TEGRA_DEVKIT_BCT_CUSTOPT_0_PERSONALITY_05 ||
        Personality == TEGRA_DEVKIT_BCT_CUSTOPT_0_PERSONALITY_C3)
        s_NvOdmQuerySdioInterfaceProperty_Whistler[2].IsCardRemovable = NV_TRUE;

    return &s_NvOdmQuerySdioInterfaceProperty_Whistler[Instance];
}

const NvOdmQueryHsmmcInterfaceProperty* NvOdmQueryGetHsmmcInterfaceProperty(NvU32 Instance)
{
    return NULL;
}

NvU32
NvOdmQueryGetBlockDeviceSectorSize(NvOdmIoModule OdmIoModule)
{
    return 0;
}

const NvOdmQueryOwrDeviceInfo* NvOdmQueryGetOwrDeviceInfo(NvU32 Instance)
{
    return &s_NvOdmQueryOwrInfo;
}

const NvOdmGpioWakeupSource *NvOdmQueryGetWakeupSources(NvU32 *pCount)
{
    *pCount = 0;
    return NULL;
}

const NvOdmQuerySpifConfig*
NvOdmQueryGetSpifConfig(
    NvU32 Instance,
    NvU32 DeviceId)
{
    return NULL;
}

NvU32
NvOdmQuerySpifWPStatus(
    NvU32 DeviceId,
    NvU32 StartBlock,
    NvU32 NumberOfBlocks)
{
    return 0;
}

NvBool NvOdmQueryGetModemId(NvU8 *ModemId)
{
    /* Only personnality == 0x2 gives modem_id equal to 1 (ie e450-108 on whistler) */
    NvU32 CustomerOption = 0;
    NvU32 Personality = 0;
    NvOdmServicesKeyListHandle hKeyList;

    hKeyList = NvOdmServicesKeyListOpen();
    if (hKeyList)
    {
        CustomerOption = NvOdmServicesGetKeyValue(hKeyList,
                                                  NvOdmKeyListId_ReservedBctCustomerOption);
        NvOdmServicesKeyListClose(hKeyList);
        Personality =
                NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, PERSONALITY, CustomerOption);
        if (Personality == 0x2)
        {
            *ModemId = 0x1;
        } else {
            *ModemId = 0x0;
        }
        return NV_TRUE;
    }
    return NV_FALSE;
}

NvBool NvOdmQueryIsClkReqPinUsed(NvU32 Instance)
{
    return NV_FALSE;
}

#ifdef LPM_BATTERY_CHARGING
NvBool NvOdmQueryChargingConfigData(NvOdmChargingConfigData *pData)
{
    return NV_FALSE;
}
#endif

NvU8
NvOdmQueryGetSdCardInstance(void)
{
    return 2;
}

NvU32
NvOdmQueryGetWifiOdmData(void)
{
    // No Odm bits reserved for Wifi Chip Selection
    return 0;
}

NvBool NvOdmQueryGetSkuOverride(NvU8 *SkuOverride)
{
    return NV_FALSE;
}

NvU32 NvOdmQueryGetOdmdataField(NvU32 CustOpt, NvU32 Field)
{
    NvU32 FieldValue = 0;

    switch (Field)
    {
    case NvOdmOdmdataField_DebugConsole:
        FieldValue = NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, CONSOLE, CustOpt);
        break;
    case NvOdmOdmdataField_DebugConsoleOptions:
        switch (NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, CONSOLE, CustOpt))
        {
        case TEGRA_DEVKIT_BCT_CUSTOPT_0_CONSOLE_DCC:
            FieldValue = NvOdmDebugConsole_Dcc;
            break;
        case TEGRA_DEVKIT_BCT_CUSTOPT_0_CONSOLE_NONE:
            FieldValue = NvOdmDebugConsole_None;
            break;
        case TEGRA_DEVKIT_BCT_CUSTOPT_0_CONSOLE_UART:
            FieldValue = NvOdmDebugConsole_UartA +
                NV_DRF_VAL(TEGRA_DEVKIT, BCT_CUSTOPT, CONSOLE_OPTION, CustOpt);
            break;
        case TEGRA_DEVKIT_BCT_CUSTOPT_0_CONSOLE_AUTOMATION:
            FieldValue = NvOdmDebugConsole_Automation;
            break;
        default:
            FieldValue = s_NvOdmQueryDebugConsoleSetting;
            break;
        }
        break;
    default:
        FieldValue = 0;
        break;
    }
    return FieldValue;
}
