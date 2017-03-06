/*
 * Copyright (c) 2007 - 2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation.  Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation
 * is strictly prohibited.
 */

#include "nvcommon.h"
#include "nvrm_structure.h"
#include "nvrm_pinmux.h"
#include "nvrm_drf.h"
#include "nvassert.h"
#include "nvrm_hwintf.h"
#include "t30/arapb_misc.h"
#include "t30/t30rm_pinmux_utils.h"
#include "nvodm_query_pinmux.h"

/*  FindConfigStart searches through an array of configuration data to find the
 *  starting position of a particular configuration in a module instance array.
 *  The stop position is programmable, so that sub-routines can be placed after
 *  the last valid true configuration */

const NvU32*
NvRmPrivT30FindConfigStart(
    const NvU32* Instance,
    NvU32 Config,
    NvU32 EndMarker)
{
    NvU32 Cnt = 0;
    while ((Cnt < Config) && (*Instance!=EndMarker))
    {
        switch (NV_DRF_VAL(MUX, ENTRY, STATE, *Instance)) {
        case PinMuxConfig_BranchLink:
        case PinMuxConfig_OpcodeExtend:
            if (*Instance==CONFIGEND())
                Cnt++;
            Instance++;
            break;
        default:
            Instance += NVRM_PINMUX_SET_OPCODE_SIZE;
            break;
        }
    }

    /* Ugly postfix.  In modules with bonafide subroutines, the last
     * configuration CONFIGEND() will be followed by a MODULEDONE()
     * token, with the first Set/Unset/Branch of the subroutine
     * following that.  To avoid leaving the "PC" pointing to a
     * MODULEDONE() in the case where the first subroutine should be
     * executed, fudge the "PC" up by one, to point to the subroutine. */
    if (EndMarker==SUBROUTINESDONE() && *Instance==MODULEDONE())
        Instance++;

    if (*Instance==EndMarker)
        Instance = NULL;

    return Instance;
}

// FIXME: this need to be moved into NvRmDeviceHandle. As of now trisate buffer
// is not bug enough to hold T30 trisate info.
static NvS16
    s_TristateRefCount[PINMUX_AUX_PEX_L2_CLKREQ_N_0 - PINMUX_AUX_ULPI_DATA0_0];

#ifndef SET_KERNEL_PINMUX
/*  NvRmPrivT30SetPadTristates will increment/decrement the reference count for
 *  each pad's global tristate value for each "ConfigSet" command in
 *  a pad group configuration, and update the register as needed */
void NvRmPrivT30SetPadTristates(
    NvRmDeviceHandle hDevice,
    const NvU32* Module,
    NvU32 Config,
    NvBool EnableTristate)
{
    int StackDepth = 0;
    const NvU32 *Instance = NULL;
    const NvU32 *ReturnStack[MAX_NESTING_DEPTH+1];

    /* The re-multiplexing configuration is stored in program 0,
     * along with the reset config. */
    if (Config==NVODM_QUERY_PINMAP_MULTIPLEXED)
        Config = 0;

    Instance = NvRmPrivT30FindConfigStart(Module, Config, MODULEDONE());
    /* The first stack return entry is NULL, so that when a ConfigEnd is
     * encountered in the "main" configuration program, we pop off a NULL
     * pointer, which causes the configuration loop to terminate. */
    ReturnStack[0] = NULL;

    /*  This loop iterates over all of the pad groups that need to be updated,
     *  and updates the reference count for each appropriately.  */

    NvOsMutexLock(hDevice->mutex);

    while (Instance)
    {
        switch (NV_DRF_VAL(MUX,ENTRY, STATE, *Instance)) {
        case PinMuxConfig_OpcodeExtend:
            /* Pop the most recent return address off of the return stack
             * (which will be NULL if no values have been pushed onto the
             * stack) */
            if (NV_DRF_VAL(MUX,ENTRY, OPCODE_EXTENSION,
                           *Instance)==PinMuxOpcode_ConfigEnd)
            {
                Instance = ReturnStack[StackDepth--];
            }
            /* ModuleDone & SubroutinesDone should never be encountered
             * during execution, for properly-formatted tables. */
            else
            {
                NV_ASSERT(0 && "Logical entry in table!\n");
            }
            break;
        case PinMuxConfig_BranchLink:
            /*  Push the next instruction onto the return stack if nesting space
                is available, and jump to the target. */
            NV_ASSERT(StackDepth<MAX_NESTING_DEPTH);
            ReturnStack[++StackDepth] = Instance+1;
            Instance = NvRmPrivT30FindConfigStart(Module,
                           NV_DRF_VAL(MUX,ENTRY,BRANCH_ADDRESS,*Instance),
                           SUBROUTINESDONE());
            NV_ASSERT(Instance && "Invalid branch configuration in table!\n");
            break;
        case PinMuxConfig_Set:
        {
            NvS16 SkipUpdate;
            NvU32 TsOffs = NV_DRF_VAL(MUX,ENTRY, MUX_REG_OFFSET, *Instance);
            NvU32 TsShift = PINMUX_AUX_ULPI_DATA0_0_TRISTATE_SHIFT;

/*  abuse pre/post-increment, to ensure that skipUpdate is 0 when the
 *  register needs to be programmed (i.e., enabling and previous value was 0,
 *  or disabling and new value is 0).
 */
            if (EnableTristate)
#if (SKIP_TRISTATE_REFCNT == 0)
                SkipUpdate =  --s_TristateRefCount[TsOffs];
            else
                SkipUpdate = s_TristateRefCount[TsOffs]++;
#else
                SkipUpdate = 1;
            else
                SkipUpdate = 0;
#endif

#if (SKIP_TRISTATE_REFCNT == 0)
            if (SkipUpdate < 0)
            {
                s_TristateRefCount[TsOffs] = 0;
                NV_DEBUG_PRINTF(("(%s:%s) Negative reference count detected "
                    "on %s, TsOffs=0x%x\n", __FILE__, __LINE__,
                    (const char*)Instance[2], TsOffs));
            }
#endif

            if (!SkipUpdate)
            {
                NvU32 Curr = NV_REGR(hDevice,
                    NVRM_MODULE_ID( NvRmModuleID_Misc, 0 ),
                    PINMUX_AUX_ULPI_DATA0_0 + 4*TsOffs);
                Curr &= ~(1<<TsShift);
#if (SKIP_TRISTATE_REFCNT == 0)
                Curr |= (EnableTristate?1:0)<<TsShift;
#endif
                NV_REGW(hDevice, NVRM_MODULE_ID( NvRmModuleID_Misc, 0 ),
                    PINMUX_AUX_ULPI_DATA0_0 + 4*TsOffs, Curr);

#if NVRM_PINMUX_DEBUG_FLAG
                NV_DEBUG_PRINTF(("Setting %s to %s\n",
                    (const char*)Instance[2],
                    (EnableTristate)?"TRISTATE" : "NORMAL"));
#endif
            }
        }
        /* fall through.
         * The "Unset" configurations are not applicable to tristate
         * configuration, so skip over them. */
        case PinMuxConfig_Unset:
            Instance += NVRM_PINMUX_SET_OPCODE_SIZE;
            break;
        }
    }
    NvOsMutexUnlock(hDevice->mutex);
}

/*  NvRmSetPinMuxCtl will apply new pin mux configurations to the pin mux
 *  control registers.  */
void
NvRmPrivT30SetPinMuxCtl(
    NvRmDeviceHandle hDevice,
    const NvU32* Module,
    NvU32 Config)
{
    NvU32 MuxRegOffset, PadMuxMask, PadMuxShift, PadMuxSet, PadMuxUnset;
    NvU32 PadInput, PadInputMask, PadInputShift;
    const NvU32 *ReturnStack[MAX_NESTING_DEPTH+1];
    const NvU32 *Instance;
    int StackDepth = 0;
    NvU32 Curr;

    ReturnStack[0] = NULL;
    Instance = Module;

    NvOsMutexLock(hDevice->mutex);

    /* The re-multiplexing configuration is stored in program 0,
     * along with the reset config. */
    if (Config==NVODM_QUERY_PINMAP_MULTIPLEXED)
        Config = 0;

    Instance = NvRmPrivT30FindConfigStart(Module, Config, MODULEDONE());

    //  Apply the new configuration, setting / unsetting as appropriate
    while (Instance)
    {
        switch (NV_DRF_VAL(MUX,ENTRY, STATE, *Instance)) {
        case PinMuxConfig_OpcodeExtend:
            if (NV_DRF_VAL(MUX,ENTRY, OPCODE_EXTENSION,
                           *Instance)==PinMuxOpcode_ConfigEnd)
            {
                Instance = ReturnStack[StackDepth--];
            }
            else
            {
                NV_ASSERT(0 && "Logical entry in table!\n");
            }
            break;
        case PinMuxConfig_BranchLink:
            NV_ASSERT(StackDepth<MAX_NESTING_DEPTH);
            ReturnStack[++StackDepth] = Instance+1;
            Instance = NvRmPrivT30FindConfigStart(Module,
                           NV_DRF_VAL(MUX,ENTRY,BRANCH_ADDRESS,*Instance),
                           SUBROUTINESDONE());
            NV_ASSERT(Instance && "Invalid branch configuration in table!\n");
            break;
        default:
        {
            MuxRegOffset = NV_DRF_VAL(MUX,ENTRY, MUX_REG_OFFSET, *Instance);
            PadMuxUnset = NV_DRF_VAL(MUX,ENTRY, PAD_MUX_UNSET, *Instance);
            PadMuxSet = NV_DRF_VAL(MUX,ENTRY, PAD_MUX_SET, *Instance);
            PadMuxMask = NV_DRF_VAL(MUX, ENTRY, PAD_MUX_MASK, *Instance);
            PadMuxShift = NV_DRF_VAL(MUX, ENTRY, PAD_MUX_SHIFT, *Instance);
            PadInput = NV_DRF_VAL(MUX, ENTRY, PAD_INPUT, *Instance);
            PadInputMask = NV_DRF_VAL(MUX, ENTRY, PAD_INPUT_MASK, *Instance);
            PadInputShift = NV_DRF_VAL(MUX, ENTRY, PAD_INPUT_SHIFT, *Instance);

            Curr = NV_REGR(hDevice, NVRM_MODULE_ID( NvRmModuleID_Misc, 0 ),
                        PINMUX_AUX_ULPI_DATA0_0 + 4*MuxRegOffset);

            if (NV_DRF_VAL(MUX,ENTRY,STATE,*Instance)==PinMuxConfig_Set)
            {
                Curr &= ~(PadMuxMask<<PadMuxShift);
                Curr |= (PadMuxSet<<PadMuxShift);
                Curr &= ~(PadInputMask<<PadInputShift);
                Curr |= (PadInput<<PadInputShift);
#if NVRM_PINMUX_DEBUG_FLAG
                NV_DEBUG_PRINTF(("Configuring %s\n",
                                 (const char *)Instance[1]));
#endif

            }
            else if (((Curr>>PadMuxShift)&PadMuxMask)==PadMuxUnset)
            {
                NV_ASSERT(NV_DRF_VAL(MUX,ENTRY,STATE,
                                     *Instance)==PinMuxConfig_Unset);
                Curr &= ~(PadMuxMask<<PadMuxShift);
                Curr |= (PadMuxSet<<PadMuxShift);
                Curr &= ~(PadInputMask<<PadInputShift);
                Curr |= (PadInput<<PadInputShift);
#if NVRM_PINMUX_DEBUG_FLAG
                NV_DEBUG_PRINTF(("Unconfiguring %s\n",
                                 (const char *)Instance[1]));
#endif
            }

            NV_REGW(hDevice, NVRM_MODULE_ID( NvRmModuleID_Misc, 0 ),
                PINMUX_AUX_ULPI_DATA0_0 + 4*MuxRegOffset, Curr);
            Instance += NVRM_PINMUX_SET_OPCODE_SIZE;
            break;
        }
        }
    }
    NvOsMutexUnlock(hDevice->mutex);
}
#endif

void NvRmPrivT30InitTrisateRefCount(NvRmDeviceHandle hDevice)
{
    NvU32 i, RegVal;

    NvOsMutexLock(hDevice->mutex);
    NvOsMemset(s_TristateRefCount, 0, sizeof(s_TristateRefCount));

    for (i=0; i<=((PINMUX_AUX_PEX_L2_CLKREQ_N_0-
                   PINMUX_AUX_ULPI_DATA0_0)>>2); i++)
    {
        RegVal = NV_REGR(hDevice, NVRM_MODULE_ID( NvRmModuleID_Misc, 0 ),
                    PINMUX_AUX_ULPI_DATA0_0 + 4*i);
        // swap from 0=normal, 1=tristate to 0=tristate, 1=normal
        RegVal = ~RegVal;

        /* the oppositely-named tristate reference count keeps track
         * of the number of active users of each pad, and
         * enables tristate when the count reaches zero. */
        s_TristateRefCount[i] =
            (NvS16)(NV_DRF_VAL(PINMUX_AUX, ULPI_DATA0, TRISTATE, RegVal) & 0x1);
    }
    NvOsMutexUnlock(hDevice->mutex);
}

void
NvRmPrivT30SetGpioTristate(
    NvRmDeviceHandle hDevice,
    NvU32 Port,
    NvU32 Pin,
    NvBool EnableTristate)
{
    NvU32 Mapping = 0;
    NvBool ret = NV_FALSE;

    NV_ASSERT(hDevice);

    if (hDevice->ChipId.Id != 0x30)
    {
        NV_ASSERT(!"Wrong chip ID");
        return;
    }

    ret = NvRmT30GetPinForGpio(hDevice, Port, Pin, &Mapping);

    if (ret)
    {
        NvS16 SkipUpdate;
        NvU32 TsOffs  = NV_DRF_VAL(MUX, GPIOMAP, TS_OFFSET, Mapping);
        NvU32 TsShift = PINMUX_AUX_ULPI_DATA0_0_TRISTATE_SHIFT;

        NvOsMutexLock(hDevice->mutex);

        if (EnableTristate)
#if (SKIP_TRISTATE_REFCNT == 0)
            SkipUpdate = --s_TristateRefCount[TsOffs];
        else
            SkipUpdate = s_TristateRefCount[TsOffs]++;
#else
            SkipUpdate = 1;
        else
            SkipUpdate = 0;
#endif

#if (SKIP_TRISTATE_REFCNT == 0)
        if (SkipUpdate < 0)
        {
            s_TristateRefCount[TsOffs] = 0;
            NV_DEBUG_PRINTF(("(%s:%s) Negative reference count detected "
            "on TsOffs=0x%x\n", __FILE__, __LINE__, TsOffs));
        }
#endif

        if (!SkipUpdate)
        {
            NvU32 Curr = NV_REGR(hDevice,
            NVRM_MODULE_ID( NvRmModuleID_Misc, 0 ),
            PINMUX_AUX_ULPI_DATA0_0 + 4*TsOffs);
            Curr &= ~(1<<TsShift);
#if (SKIP_TRISTATE_REFCNT == 0)
            Curr |= (EnableTristate?1:0)<<TsShift;
#endif
            NV_REGW(hDevice, NVRM_MODULE_ID( NvRmModuleID_Misc, 0 ),
            PINMUX_AUX_ULPI_DATA0_0 + 4*TsOffs, Curr);
        }

        NvOsMutexUnlock(hDevice->mutex);
    }
}
