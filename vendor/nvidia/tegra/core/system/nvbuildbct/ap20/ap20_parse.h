/**
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

/*
 * set.h - Definitions for the buildimage state setting code.
 */

/*
 * TODO / Notes
 * - Add doxygen commentary
 */

#ifndef INCLUDED_AP20_PARSE_H
#define INCLUDED_AP20_PARSE_H


#include "../nvbuildbct.h"

#if defined(__cplusplus)
extern "C"
{
#endif

void Ap20ProcessConfigFile(BuildBctContext *Context);

#if defined(__cplusplus)
}
#endif

#endif /* #ifndef INCLUDED_SET_H */
