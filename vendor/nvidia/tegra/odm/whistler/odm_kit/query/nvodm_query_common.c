/*
 * Copyright (c) 2009-2012 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
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
#include "nvodm_services.h"
#include "tegra_devkit_custopt.h"
#include "nvodm_keylist_reserved.h"
#include "nvrm_drf.h"

/* --- Function Implementations ---*/

static NvU32
GetBctKeyValue(void)
{
    NvOdmServicesKeyListHandle hKeyList = NULL;
    NvU32 BctCustOpt = 0;

    hKeyList = NvOdmServicesKeyListOpen();
    if (hKeyList)
    {
        BctCustOpt = NvOdmServicesGetKeyValue(hKeyList,
                         NvOdmKeyListId_ReservedBctCustomerOption);
        NvOdmServicesKeyListClose(hKeyList);
    }
    return BctCustOpt;
}

/**
 * This function is called from early boot process.
 * Therefore, it cannot use global variables.
 */
NvU32 NvOdmQueryOsMemSize(NvOdmMemoryType MemType, const NvOdmOsOsInfo *pOsInfo)
{
    NvU32 SdramSize;
    NvU32 SdramBctCustOpt;

    if (pOsInfo == NULL)
    {
        return 0;
    }

    switch (MemType)
    {
        case NvOdmMemoryType_Sdram:
            SdramBctCustOpt = GetBctKeyValue();

            switch (NV_DRF_VAL(TEGRA_DEVKIT, BCT_SYSTEM, MEMORY, SdramBctCustOpt))
            {
                case TEGRA_DEVKIT_BCT_SYSTEM_0_MEMORY_1:
                    SdramSize = 0x10000000; // 256 MB
                    break;

                case TEGRA_DEVKIT_BCT_SYSTEM_0_MEMORY_DEFAULT:
                case TEGRA_DEVKIT_BCT_SYSTEM_0_MEMORY_2:
                default:
                    SdramSize = 0x20000000; // 512 MB
                    break;

                case TEGRA_DEVKIT_BCT_SYSTEM_0_MEMORY_3:
                    SdramSize = 0x40000000; // 1 GB
                    break;
            }

            return SdramSize;

        case NvOdmMemoryType_Nor:
            return 0x00400000;  // 4 MB

        case NvOdmMemoryType_Nand:
        case NvOdmMemoryType_I2CEeprom:
        case NvOdmMemoryType_Hsmmc:
        case NvOdmMemoryType_Mio:
        default:
            return 0;
    }
}

NvU32 NvOdmQueryMemSize(NvOdmMemoryType MemType)
{
    NvOdmOsOsInfo Info;

    // Get the information about the calling OS.
    (void)NvOdmOsGetOsInformation(&Info);

    return NvOdmQueryOsMemSize(MemType, &Info);
}

NvU32 NvOdmQueryCarveoutSize(void)
{
    NvU32 CarveBctCustOpt = GetBctKeyValue();

    switch (NV_DRF_VAL(TEGRA_DEVKIT, BCT_CARVEOUT, MEMORY, CarveBctCustOpt))
    {
        case TEGRA_DEVKIT_BCT_CARVEOUT_0_MEMORY_1:
            return 0x00400000;// 4MB
        case TEGRA_DEVKIT_BCT_CARVEOUT_0_MEMORY_2:
            return 0x00800000;// 8MB
        case TEGRA_DEVKIT_BCT_CARVEOUT_0_MEMORY_3:
            return 0x00C00000;// 12MB
        case TEGRA_DEVKIT_BCT_CARVEOUT_0_MEMORY_4:
            return 0x01000000;// 16MB
        case TEGRA_DEVKIT_BCT_CARVEOUT_0_MEMORY_5:
            return 0x01400000;// 20MB
        case TEGRA_DEVKIT_BCT_CARVEOUT_0_MEMORY_6:
            return 0x01800000;// 24MB
        case TEGRA_DEVKIT_BCT_CARVEOUT_0_MEMORY_7:
            return 0x01C00000;// 28MB
        case TEGRA_DEVKIT_BCT_CARVEOUT_0_MEMORY_8:
            return 0x02000000; // 32 MB
        case TEGRA_DEVKIT_BCT_CARVEOUT_0_MEMORY_9:
            return 0x03000000; // 48 MB
        case TEGRA_DEVKIT_BCT_CARVEOUT_0_MEMORY_A:
            return 0x04000000; // 64 MB
        case TEGRA_DEVKIT_BCT_CARVEOUT_0_MEMORY_B:
            return 0x08000000; // 128 MB
        case TEGRA_DEVKIT_BCT_CARVEOUT_0_MEMORY_C:
            return 0x10000000; // 256 MB
        case TEGRA_DEVKIT_BCT_CARVEOUT_0_MEMORY_DEFAULT:
        default:
            return 0x04000000; // 64 MB
    }
}

NvU32 NvOdmQuerySecureRegionSize(void)
{
#ifdef CONFIG_TRUSTED_FOUNDATIONS
    return 0x200000; // 2 MB
#else
    return 0x800000; //8 MB
#endif
}

NvU32 NvOdmQueryDataPartnEncryptFooterSize(const char *name)
{
    return 0;
}
