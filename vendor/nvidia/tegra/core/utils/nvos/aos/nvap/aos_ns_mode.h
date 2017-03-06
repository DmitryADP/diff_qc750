/*
 * Copyright (c) 2012 NVIDIA Corporation.  All Rights Reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation.  Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation
 * is strictly prohibited.
 */

#ifndef INCLUDED_AOS_NS_MODE_H
#define INCLUDED_AOS_NS_MODE_H

#include "nvcommon.h"
#include "aos.h"
#include "cpu.h"

void    nvaosNSModeInit(void);
void    nvaosSetNonSecureStateMMU(void);

#endif
