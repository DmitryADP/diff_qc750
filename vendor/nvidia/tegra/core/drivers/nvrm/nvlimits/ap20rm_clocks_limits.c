/*
 * Copyright (c) 2009 - 2011 NVIDIA Corporation.  All Rights Reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation.  Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation
 * is strictly prohibited.
 */

#include "nvcommon.h"
#include "nvassert.h"
#include "nvrm_drf.h"
#include "nvrm_hwintf.h"
#include "nvrm_clocks.h"
#include "ap20rm_clocks_limits_private.h"
#include "ap15rm_private.h"
#include "ap20/arapb_misc.h"
#include "ap20/arfuse_public.h"
#include "ap20/arclk_rst.h"

/*
 * Core voltage levels SoC clock frequency limits were characterized at
 * for each process corner. Combined pointer table arranged in order of
 * process calibration settings
 */
static const NvU32 s_Ap20SS_ShmooVoltages[] = { NV_AP20SS_SHMOO_VOLTAGES };
static const NvU32 s_Ap20ST_ShmooVoltages[] = { NV_AP20ST_SHMOO_VOLTAGES };
static const NvU32 s_Ap20FT_ShmooVoltages[] = { NV_AP20FT_SHMOO_VOLTAGES };
static const NvU32 s_Ap20FF_ShmooVoltages[] = { NV_AP20FF_SHMOO_VOLTAGES };
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_ShmooVoltages) == NV_ARRAY_SIZE(s_Ap20ST_ShmooVoltages));
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_ShmooVoltages) == NV_ARRAY_SIZE(s_Ap20FT_ShmooVoltages));
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_ShmooVoltages) == NV_ARRAY_SIZE(s_Ap20FF_ShmooVoltages));
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_ShmooVoltages) <= NVRM_VOLTAGE_STEPS);

static const NvU32* s_pAp20ShmooVoltages[] =
{
    &s_Ap20SS_ShmooVoltages[0],     /* Calibration setting 0 - SS corner */
    &s_Ap20ST_ShmooVoltages[0],     /* Calibration setting 1 - ST corner */
    &s_Ap20FT_ShmooVoltages[0],     /* Calibration setting 2 - FT corner */
    &s_Ap20FF_ShmooVoltages[0]      /* Calibration setting 3 - FF corner */
};
NV_CT_ASSERT(NV_ARRAY_SIZE(s_pAp20ShmooVoltages) == NV_AP20_PROCESS_CORNERS);

/*
 * Module clock frequency limits table generated by h/w characterization for
 * each process corner (each table entry includes h/w device id and applies
 * to all instances of the respective device). Combined pointer table arranged
 * in order of process calibration settings
 */
static const NvRmScaledClkLimits s_Ap20SS_ScaledClkLimits[] = { NV_AP20SS_SCALED_CLK_LIMITS };
static const NvRmScaledClkLimits s_Ap20ST_ScaledClkLimits[] = { NV_AP20ST_SCALED_CLK_LIMITS };
static const NvRmScaledClkLimits s_Ap20FT_ScaledClkLimits[] = { NV_AP20FT_SCALED_CLK_LIMITS };
static const NvRmScaledClkLimits s_Ap20FF_ScaledClkLimits[] = { NV_AP20FF_SCALED_CLK_LIMITS };
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_ScaledClkLimits) == NV_ARRAY_SIZE(s_Ap20ST_ScaledClkLimits));
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_ScaledClkLimits) == NV_ARRAY_SIZE(s_Ap20FT_ScaledClkLimits));
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_ScaledClkLimits) == NV_ARRAY_SIZE(s_Ap20FF_ScaledClkLimits));

static const NvRmScaledClkLimits* s_pAp20ScaledClkLimits[] =
{
    &s_Ap20SS_ScaledClkLimits[0],     /* Calibration setting 0 - SS corner */
    &s_Ap20ST_ScaledClkLimits[0],     /* Calibration setting 1 - ST corner */
    &s_Ap20FT_ScaledClkLimits[0],     /* Calibration setting 2 - FT corner */
    &s_Ap20FF_ScaledClkLimits[0]      /* Calibration setting 3 - FF corner */
};
NV_CT_ASSERT(NV_ARRAY_SIZE(s_pAp20ScaledClkLimits) == NV_AP20_PROCESS_CORNERS);

/*
 * CPU voltage levels used for CPU frequency limits characterization and the
 * respective characterization results for all CPU process corners (arranged
 * in order of process calibration settings)
 */

// A01 and A02 tables
static const NvU32 s_Ap20SS_CpuVoltages[] = { NV_AP20SS_CPU_VOLTAGES };
static const NvU32 s_Ap20ST_CpuVoltages[] = { NV_AP20ST_CPU_VOLTAGES };
static const NvU32 s_Ap20FT_CpuVoltages[] = { NV_AP20FT_CPU_VOLTAGES };
static const NvU32 s_Ap20FF_CpuVoltages[] = { NV_AP20FF_CPU_VOLTAGES };
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_CpuVoltages) == NV_ARRAY_SIZE(s_Ap20ST_CpuVoltages));
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_CpuVoltages) == NV_ARRAY_SIZE(s_Ap20FT_CpuVoltages));
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_CpuVoltages) == NV_ARRAY_SIZE(s_Ap20FF_CpuVoltages));
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_CpuVoltages) <= NVRM_VOLTAGE_STEPS);

static const NvU32* s_pAp20CpuVoltages[] =
{
    &s_Ap20SS_CpuVoltages[0],     /* Calibration setting 0 - SS CPU corner */
    &s_Ap20ST_CpuVoltages[0],     /* Calibration setting 1 - ST CPU corner */
    &s_Ap20FT_CpuVoltages[0],     /* Calibration setting 2 - FT CPU corner */
    &s_Ap20FF_CpuVoltages[0],     /* Calibration setting 3 - FF CPU corner */
};
NV_CT_ASSERT(NV_ARRAY_SIZE(s_pAp20CpuVoltages) == NV_AP20_PROCESS_CORNERS);

static const NvRmScaledClkLimits s_Ap20ScaledCpuClkLimits[] =
{
    NV_AP20SS_SCALED_CPU_CLK_LIMITS,
    NV_AP20ST_SCALED_CPU_CLK_LIMITS,
    NV_AP20FT_SCALED_CPU_CLK_LIMITS,
    NV_AP20FF_SCALED_CPU_CLK_LIMITS,
};
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20ScaledCpuClkLimits) == NV_AP20_PROCESS_CORNERS);

// A03+ tables
static const NvU32 s_Ap20SS_A03CpuVoltages[] = { NV_AP20SS_A03CPU_VOLTAGES };
static const NvU32 s_Ap20ST_A03CpuVoltages[] = { NV_AP20ST_A03CPU_VOLTAGES };
static const NvU32 s_Ap20FT_A03CpuVoltages[] = { NV_AP20FT_A03CPU_VOLTAGES };
static const NvU32 s_Ap20FF_A03CpuVoltages[] = { NV_AP20FF_A03CPU_VOLTAGES };
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_A03CpuVoltages) == NV_ARRAY_SIZE(s_Ap20ST_A03CpuVoltages));
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_A03CpuVoltages) == NV_ARRAY_SIZE(s_Ap20FT_A03CpuVoltages));
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_A03CpuVoltages) == NV_ARRAY_SIZE(s_Ap20FF_A03CpuVoltages));
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20SS_A03CpuVoltages) <= NVRM_VOLTAGE_STEPS);

static const NvU32* s_pAp20_A03CpuVoltages[] =
{
    &s_Ap20SS_A03CpuVoltages[0],     /* Calibration setting 0 - SS CPU corner */
    &s_Ap20ST_A03CpuVoltages[0],     /* Calibration setting 1 - ST CPU corner */
    &s_Ap20FT_A03CpuVoltages[0],     /* Calibration setting 2 - FT CPU corner */
    &s_Ap20FF_A03CpuVoltages[0],     /* Calibration setting 3 - FF CPU corner */
};
NV_CT_ASSERT(NV_ARRAY_SIZE(s_pAp20_A03CpuVoltages) == NV_AP20_PROCESS_CORNERS);

static const NvRmScaledClkLimits s_Ap20Scaled_A03CpuClkLimits[] =
{
    NV_AP20SS_SCALED_A03CPU_CLK_LIMITS,
    NV_AP20ST_SCALED_A03CPU_CLK_LIMITS,
    NV_AP20FT_SCALED_A03CPU_CLK_LIMITS,
    NV_AP20FF_SCALED_A03CPU_CLK_LIMITS,
};
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20Scaled_A03CpuClkLimits) == NV_AP20_PROCESS_CORNERS);

static const NvRmScaledClkLimits s_Ap25Scaled_A03CpuClkLimits[] =
{
    NV_AP20SS_SCALED_A03CPU_CLK_LIMITS,
    NV_AP20ST_SCALED_A03CPU_CLK_LIMITS,
    NV_AP25FT_SCALED_A03CPU_CLK_LIMITS,
    NV_AP20FF_SCALED_A03CPU_CLK_LIMITS,
};
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap25Scaled_A03CpuClkLimits) == NV_AP20_PROCESS_CORNERS);


/*
 * Maximum frequency limits table for clocks depended on SoC SKU;
 * ordered by SKU number
 */
static const struct
{
    NvRmSKUedLimits limits;
    NvU32 sku;
} s_Ap20SKUedLimits[] =
{
    NV_AP20_SKUED_LIMITS
};

/**
 * OSC Doubler tap delays for each supported frequency in order
 * of process calibration settings
 */
static const NvRmOscDoublerConfig s_Ap20OscDoublerConfigurations[] =
{
    NV_AP20_OSC_DOUBLER_CONFIGURATIONS
};

/**
 * EMC DQSIB offsets in order of process calibration settings
 */
static const NvU32 s_Ap20DqsibOffsets[] =
{
    NV_AP20_DQSIB_OFFSETS
};
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20DqsibOffsets) == NV_AP20_PROCESS_CORNERS);
NV_CT_ASSERT(NV_AP20_PROCESS_CORNERS <= NVRM_PROCESS_CORNERS);

static const NvU32 s_Ap25DqsibOffsets[] =
{
    NV_AP25_DQSIB_OFFSETS
};
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap25DqsibOffsets) == NV_AP20_PROCESS_CORNERS);

/**
 * Speedo thresholds
 */
static const NvU32 s_Ap20_A02SpeedoLimits[] = { NV_AP20_A02_SPEEDO_LIMITS };
static const NvU32 s_Ap20_A03SpeedoLimits[] = { NV_AP20_A03_SPEEDO_LIMITS };
static const NvU32 s_Ap25_A03SpeedoLimits[] = { NV_AP25_A03_SPEEDO_LIMITS };
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20_A02SpeedoLimits) == NV_AP20_PROCESS_CORNERS);
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20_A03SpeedoLimits) == NV_AP20_PROCESS_CORNERS);
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap25_A03SpeedoLimits) == NV_AP20_PROCESS_CORNERS);

static const NvU32 s_Ap20_A02CpuSpeedoLimits[] = { NV_AP20_A02_CPU_SPEEDO_LIMITS };
static const NvU32 s_Ap20_A03CpuSpeedoLimits[] = { NV_AP20_A03_CPU_SPEEDO_LIMITS };
static const NvU32 s_Ap25_A03CpuSpeedoLimits[] = { NV_AP25_A03_CPU_SPEEDO_LIMITS };
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20_A02CpuSpeedoLimits) == NV_AP20_PROCESS_CORNERS);
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap20_A03CpuSpeedoLimits) == NV_AP20_PROCESS_CORNERS);
NV_CT_ASSERT(NV_ARRAY_SIZE(s_Ap25_A03CpuSpeedoLimits) == NV_AP20_PROCESS_CORNERS);

/*
 * Combined SoC characterization data record
 */
static NvRmSocShmoo s_Ap20SocShmoo = {0};
static NvRmCpuShmoo s_Ap20CpuShmoo = {0};

void NvRmPrivAp20FlavorInit(
    NvRmDeviceHandle hRmDevice,
    NvRmChipFlavor* pChipFlavor)
{
    NvU32 i;
    NvBool IsA03Plus = NV_FALSE;
#if (NVCPU_IS_X86 && NVOS_IS_WINDOWS)
    pChipFlavor->sku = 0;
    pChipFlavor->corner = 0;
    pChipFlavor->CpuCorner = 0;
#else
    // Read chip ID
    NvRmChipId ChipId;
    NvU32 reg = NV_REGR(hRmDevice, NVRM_MODULE_ID(NvRmModuleID_Misc, 0),
        APB_MISC_GP_HIDREV_0);
    ChipId.Id = (NvU16)NV_DRF_VAL(APB_MISC_GP, HIDREV, CHIPID, reg );
    ChipId.Major = (NvU8)NV_DRF_VAL(APB_MISC_GP, HIDREV, MAJORREV, reg );
    ChipId.Minor = (NvU8)NV_DRF_VAL(APB_MISC_GP, HIDREV, MINORREV, reg );
    if (ChipId.Minor >= 3)
        IsA03Plus = NV_TRUE;

    if ((ChipId.Major != 0) && (ChipId.Id == 0x20))
    {
        NvU32 freg, creg, speedo;
        const NvU32* pSpeedoLimits = NULL;
        const NvU32* pCpuSpeedoLimits = NULL;

        // Made fuses visible
        creg = NV_REGR(hRmDevice,
            NVRM_MODULE_ID(NvRmPrivModuleID_ClockAndReset, 0),
            CLK_RST_CONTROLLER_MISC_CLK_ENB_0);
        creg = NV_FLD_SET_DRF_NUM(
            CLK_RST_CONTROLLER, MISC_CLK_ENB, CFG_ALL_VISIBLE, 1, creg);
        NV_REGW(hRmDevice,
            NVRM_MODULE_ID(NvRmPrivModuleID_ClockAndReset, 0),
            CLK_RST_CONTROLLER_MISC_CLK_ENB_0, creg);

        // Read chip sku and select the respective speedo thresholds
        freg =  NV_REGR(hRmDevice, NVRM_MODULE_ID( NvRmModuleID_Fuse, 0 ),
                        FUSE_SKU_INFO_0);
        pChipFlavor->sku = (NvU16)NV_DRF_VAL(FUSE, SKU_INFO, SKU_INFO, freg);

        if (IsAp25Speedo(pChipFlavor->sku))
        {
            NV_ASSERT(IsA03Plus);
            pSpeedoLimits = &s_Ap25_A03SpeedoLimits[0];
            pCpuSpeedoLimits = &s_Ap25_A03CpuSpeedoLimits[0];
        }
        else if (IsA03Plus)
        {
            pSpeedoLimits = &s_Ap20_A03SpeedoLimits[0];
            pCpuSpeedoLimits = &s_Ap20_A03CpuSpeedoLimits[0];
        }
        else
        {
            pSpeedoLimits = &s_Ap20_A02SpeedoLimits[0];
            pCpuSpeedoLimits = &s_Ap20_A02CpuSpeedoLimits[0];
        }

        // Read core speedo fuses from spare bits 47-40 (ORed with redundancy
        // bits 55-48), set core process corner based on the speedo thresholds
        for (speedo = 0, i = 0; i < 8; i++)
        {
            freg = NV_REGR(hRmDevice, NVRM_MODULE_ID(NvRmModuleID_Fuse, 0),
                           FUSE_SPARE_BIT_47_0 - (i * 4)) |
                   NV_REGR(hRmDevice, NVRM_MODULE_ID(NvRmModuleID_Fuse, 0),
                           FUSE_SPARE_BIT_55_0 - (i * 4));
            speedo = (speedo << 1) | (freg & 0x1);
        }
        speedo *= 4;    // 4:1 speedo scale

        for (i = 0; i < NV_AP20_PROCESS_CORNERS; i++)
        {
            if (speedo < pSpeedoLimits[i])
                break;
        }
        pChipFlavor->corner = i;

        // Read CPU speedo fuses from spare bits 29-20 (ORed with redundancy
        // bits 39-30), set CPU process corner based on the speedo thresholds
        for (speedo = 0, i = 0; i < 10; i++)
        {
            freg = NV_REGR(hRmDevice, NVRM_MODULE_ID(NvRmModuleID_Fuse, 0),
                           FUSE_SPARE_BIT_29_0 - (i * 4)) |
                   NV_REGR(hRmDevice, NVRM_MODULE_ID(NvRmModuleID_Fuse, 0),
                           FUSE_SPARE_BIT_39_0 - (i * 4));
            speedo = (speedo << 1) | (freg & 0x1);
        }
        speedo *= 4;    // 4:1 speedo scale

        for (i = 0; i < NV_AP20_PROCESS_CORNERS; i++)
        {
            if (speedo < pCpuSpeedoLimits[i])
                break;
        }
        pChipFlavor->CpuCorner = i;

        // Mask fuse access
        creg = NV_FLD_SET_DRF_NUM(
            CLK_RST_CONTROLLER, MISC_CLK_ENB, CFG_ALL_VISIBLE, 0, creg);
        NV_REGW(hRmDevice,
            NVRM_MODULE_ID(NvRmPrivModuleID_ClockAndReset, 0),
            CLK_RST_CONTROLLER_MISC_CLK_ENB_0, creg);
    }
    else if (ChipId.Id == 0x20)
    {
        pChipFlavor->sku = 0;
        pChipFlavor->corner = 0;
        pChipFlavor->CpuCorner = 0;
    }
    else
    {
        NV_ASSERT(!"Invalid chip ID");
        pChipFlavor->sku = 0;
        pChipFlavor->corner = 0;
        pChipFlavor->CpuCorner = 0;
        pChipFlavor->pSocShmoo = NULL;
        pChipFlavor->pCpuShmoo = NULL;
        return;
    }
#endif
    NV_ASSERT(pChipFlavor->corner < NV_AP20_PROCESS_CORNERS);
    NV_ASSERT(pChipFlavor->CpuCorner < NV_AP20_PROCESS_CORNERS);
    NvOsMemset((void*)&s_Ap20SocShmoo, 0, sizeof(s_Ap20SocShmoo));
    NvOsMemset((void*)&s_Ap20CpuShmoo, 0, sizeof(s_Ap20CpuShmoo));

    // Fill in SKUed limits for the chip at hand
    for (i = 0; i < NV_ARRAY_SIZE(s_Ap20SKUedLimits); i++)
    {
        if (s_Ap20SKUedLimits[i].sku == pChipFlavor->sku)
            break;
    }
    NV_ASSERT(i < NV_ARRAY_SIZE(s_Ap20SKUedLimits));
    s_Ap20SocShmoo.pSKUedLimits = &s_Ap20SKUedLimits[i].limits;

    // Fill in core rail characterization data for the chip at hand
    s_Ap20SocShmoo.ShmooVoltages = s_pAp20ShmooVoltages[pChipFlavor->corner];
    for (i = 0; i < NV_ARRAY_SIZE(s_Ap20SS_ShmooVoltages); i++)
    {
        if (s_Ap20SocShmoo.ShmooVoltages[i] ==
            s_Ap20SocShmoo.pSKUedLimits->NominalCoreMv)
            break;
    }
    if(i >= NV_ARRAY_SIZE(s_Ap20SS_ShmooVoltages))
    {
        NV_ASSERT(!"Nominal voltage not found");
        i = NV_ARRAY_SIZE(s_Ap20SS_ShmooVoltages) - 1;  // use max shmoo volt
    }
    s_Ap20SocShmoo.ShmooVmaxIndex = i;

    s_Ap20SocShmoo.ScaledLimitsList =
        s_pAp20ScaledClkLimits[pChipFlavor->corner];
    s_Ap20SocShmoo.ScaledLimitsListSize =
        NV_ARRAY_SIZE(s_Ap20SS_ScaledClkLimits);

    s_Ap20SocShmoo.OscDoublerCfgList =
        &s_Ap20OscDoublerConfigurations[0];
    s_Ap20SocShmoo.OscDoublerCfgListSize =
        NV_ARRAY_SIZE(s_Ap20OscDoublerConfigurations);

    NV_ASSERT(pChipFlavor->corner < NV_ARRAY_SIZE(s_Ap20DqsibOffsets));
    s_Ap20SocShmoo.DqsibOffset = IsAp25Speedo(pChipFlavor->sku) ?
        s_Ap25DqsibOffsets[pChipFlavor->corner] :
        s_Ap20DqsibOffsets[pChipFlavor->corner];

    s_Ap20SocShmoo.SvopLowVoltage = NV_AP20_SVOP_LOW_VOLTAGE;
    if (pChipFlavor->corner == 0)   // Apply low setting only to SS parts
        s_Ap20SocShmoo.SvopLowSetting = NV_AP20_SVOP_LOW_SETTING;
    else
        s_Ap20SocShmoo.SvopLowSetting = NV_AP20_SVOP_HIGH_SETTING;
    s_Ap20SocShmoo.SvopHighSetting = NV_AP20_SVOP_HIGH_SETTING;

    pChipFlavor->pSocShmoo = &s_Ap20SocShmoo;

    // Fill in dedicated CPU rail characterization data for the chip at hand
    s_Ap20CpuShmoo.ShmooVoltages = IsA03Plus ?
        s_pAp20_A03CpuVoltages[pChipFlavor->CpuCorner] :
        s_pAp20CpuVoltages[pChipFlavor->CpuCorner];
    s_Ap20CpuShmoo.pScaledCpuLimits = IsAp25Speedo(pChipFlavor->sku) ?
        &s_Ap25Scaled_A03CpuClkLimits[pChipFlavor->CpuCorner] : (IsA03Plus ?
        &s_Ap20Scaled_A03CpuClkLimits[pChipFlavor->CpuCorner] :
        &s_Ap20ScaledCpuClkLimits[pChipFlavor->CpuCorner]);
    s_Ap20CpuShmoo.ShmooVmaxIndex = IsA03Plus ?
        (NV_ARRAY_SIZE(s_Ap20SS_A03CpuVoltages) -1) :
        (NV_ARRAY_SIZE(s_Ap20SS_CpuVoltages) -1);

    for (i = 0; i < s_Ap20CpuShmoo.ShmooVmaxIndex; i++)
    {
        if ((s_Ap20CpuShmoo.pScaledCpuLimits->MaxKHzList[i + 1] == 0) ||
            (s_Ap20CpuShmoo.pScaledCpuLimits->MaxKHzList[i] >=
             s_Ap20SocShmoo.pSKUedLimits->CpuMaxKHz))
            break;
    }
    s_Ap20CpuShmoo.ShmooVmaxIndex = i;
    NV_ASSERT(s_Ap20CpuShmoo.ShmooVoltages[i] <=
              s_Ap20SocShmoo.pSKUedLimits->NominalCpuMv);
    NV_ASSERT(s_Ap20CpuShmoo.pScaledCpuLimits->MaxKHzList[i] >=
              s_Ap20SocShmoo.pSKUedLimits->CpuMaxKHz);

    pChipFlavor->pCpuShmoo = &s_Ap20CpuShmoo;
}

