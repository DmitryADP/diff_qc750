/*
 * Copyright (c) 2012 NVIDIA Corporation.  All Rights Reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation.  Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation
 * is strictly prohibited.
 */

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <qnx/nvdisp_devctls.h>
#include <nvdc.h>
#include "nvdc_priv.h"

int nvdcQueryNumHeads(struct nvdcState *state)
{
    int i = 0;

    if (state == NULL)
        return -EINVAL;

    while (state->dispFd[i] >= 0 && i < NVDC_MAX_HEADS) {
        i++;
    }

    nvdc_info("%s passed (#Heads = %d)\n",__func__, i);
    return i;
}

/*
 * Initialize the displays array.  Called once at startup, outputs aren't
 * expected to change
 */
int _nvdcInitOutputInfo(struct nvdcState *state)
{
    unsigned int num, i;
    int ret;

    ret = devctl(state->ctrlFd, NVDISP_CTRL_GET_NUM_OUTPUTS,
                 &num, sizeof(num), NULL);
    if (ret != EOK) {
        nvdc_error("%s: devctl failure\n", __func__);
        return ret;
    }

    if (num > NVDC_MAX_OUTPUTS) {
        return E2BIG;
    }
    state->numOutputs = num;

    for (i = 0; i < num; i++) {
        struct nvdisp_output_prop prop = {};

        prop.handle = i;

        ret = devctl(state->ctrlFd, NVDISP_CTRL_GET_OUTPUT_PROPERTIES, &prop,
                     sizeof(prop), NULL);
        if (ret != EOK) {
            nvdc_error("%s devctl failure\n",__func__);
            return ret;
        }
        state->displays[i].dcHandle = i;
    }

    nvdc_info("# Ouputs = %d\n", num);
    return ret;
}

int nvdcQueryDisplays(struct nvdcState *state,
                      nvdcDisplayHandle **pDisplays,
                      int *nDisplays)
{
    nvdcDisplayHandle *displays;
    unsigned int num = state->numOutputs, i;

    if ((pDisplays == NULL) || (state == NULL) || (nDisplays == NULL))
            return EINVAL;

    displays = malloc(sizeof(*displays) * num);

    if (displays == NULL) {
        nvdc_error("%s: no memory\n",__func__);
        return ENOMEM;
    }

    for (i = 0; i < num; i++) {
        /*
         * These are opaque handles to clients.  We just use a pointer into our
         * displays array for ease of fetching it back out later.
         */
        displays[i] = &state->displays[i];
    }

    *nDisplays = num;
    *pDisplays = displays;

    nvdc_info("%s passed\n",__func__);
    return 0;
}

int nvdcQueryDisplayInfo(struct nvdcState *state,
                         struct nvdcDisplay *display,
                         struct nvdcDisplayInfo *info)
{
    struct nvdisp_output_prop prop = {};
    int ret;

    if ((state == NULL) || (display == NULL) || (info == NULL))
        return EINVAL;

    prop.handle = display->dcHandle;
    ret = devctl(state->ctrlFd, NVDISP_CTRL_GET_OUTPUT_PROPERTIES,
                 &prop, sizeof(prop), NULL);
    if (ret != EOK) {
        return ret;
    }

    info->type = prop.type;
    info->boundHead = prop.assosciated_head;
    info->connected = prop.connected;
    info->headMask = prop.head_mask;
    nvdc_info("Type = %d, Connected = %x, Mask = %x\n",
            prop.type,
            prop.assosciated_head,
            prop.head_mask);
    return 0;
}

int nvdcQueryDisplayEdid(struct nvdcState *state,
                         struct nvdcDisplay *display,
                         void **data,
                         size_t *len)
{
    nvdc_error("%s unimplemented\n",__func__);
    return ENOANO;
}

int nvdcDisplayBind(struct nvdcState *state,
                    struct nvdcDisplay *display,
                    int head)
{
    /*
     * XXX flesh this out with an actual implementation.  Right now we assume
     * that DC handle n is permanently bound to head n.
     */
    nvdc_error("%s unimplemented\n",__func__);
    return (head == display->dcHandle) ? 0 : ENOANO;
}

int nvdcDisplayUnbind(struct nvdcState *state,
                      struct nvdcDisplay *display,
                      int head)
{
    /*
     * XXX flesh this out with an actual implementation.  Right now we assume
     * that DC handle n is permanently bound to head n, unbinding is not
     * supported.
     */
    nvdc_error("%s unimplemented\n",__func__);
    return ENOANO;
}

int nvdcQueryVblankSyncpt(struct nvdcState *state, int head,
                          unsigned int *syncpt)
{
    nvdc_error("%s unimplemented\n",__func__);
    return ENOANO;
}
