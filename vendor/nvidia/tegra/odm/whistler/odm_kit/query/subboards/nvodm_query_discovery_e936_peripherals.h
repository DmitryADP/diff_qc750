/*
 * Copyright (c) 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

/**
 * @file
 * <b>NVIDIA APX ODM Kit::
 *         Implementation of the ODM Peripheral Discovery API</b>
 *
 * @b Description: Specifies the peripheral connectivity database Peripheral
 *                 entries for the E936 ISDB-T module.
 */

// Murata tuner source
{
    NV_ODM_GUID('m','u','r','a','t','a','5','7'),
    s_ffaVIPBitstreamAddresses,
    NV_ARRAY_SIZE(s_ffaVIPBitstreamAddresses),
    NvOdmPeripheralClass_Other
},

// NOTE: This list *must* end with a trailing comma.
