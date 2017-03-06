/**
 * Copyright (c) 2008-2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */


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
#include <ov5650.h>
#endif

#include "sensor_bayer_ov5650.h"
#include "imager_util.h"
#include "focuser_sh532u.h"
#include "nvodm_query_gpio.h"

#ifndef DEBUG_PRINTS
#define DEBUG_PRINTS (0)
#endif

#ifndef BUILD_FOR_VENTANA
#define BUILD_FOR_VENTANA (0)
#endif

#ifndef BUILD_FOR_WHISTLER
#define BUILD_FOR_WHISTLER (0)
#endif

#ifndef BUILD_FOR_CARDHU
#define BUILD_FOR_CARDHU (0)
#endif

#if (BUILD_FOR_VENTANA == 1)
#include "sensor_bayer_ov5650_config_ventana.h"
#else
#include "sensor_bayer_ov5650_config_cardhu.h"
#endif

#ifndef NV_ENABLE_DEBUG_PRINTS
#define NV_ENABLE_DEBUG_PRINTS (0)
#endif

// the standard assert doesn't help us
#undef NV_ASSERT
#define NV_ASSERT(x) \
    do{if(!(x)){NvOsDebugPrintf("ASSERT at %s:%d\n", __FILE__,__LINE__);}}while(0)

/****************************************************************************
 * Implementing for a New Sensor:
 *
 * When implementing for a new sensor from this template, there are four
 * phases:
 * Phase 0: Gather sensor information. In phase 0, we should gather the sensor
 *          information for bringup such as sensor capabilities, set mode i2c
 *          write sequence, NvOdmIoAddress in nvodm_query_discovery_imager.h,
 *          and so on.
 * Phase 1: Bring up a sensor. After phase 1, we should be able to do a basic
 *          still capture with minimal interactions between camera driver
 *          and the sensor
 * Phase 2: Calibrate a sensor. After phase 2, we should be able to calibrate
 *          a sensor. Calibration will need some interactions between camera
 *          driver and the sensor such as setting exposure, gains, and so on.
 * Phase 3: Fully functioning sensor. After phase 3, the sensor should be fully
 *          functioning as described in nvodm_imager.h
 *
 * To help implementing for a new sensor, the template code will be marked as
 * Phase 0, 1, 2, or 3.
 ****************************************************************************/
/**
 * Sensor default settings. Phase 2. Sensor Dependent.
 * #defines make the code more readable and helps maintainability
 * The template tries to keep the list reduced to just things that
 * are used more than once.  Ideally, all numbers in the code would use
 * a #define.
 */
// manual exposure control
//
#define SENSOR_BAYER_DEFAULT_PLL_MULT            (0x0050) // refer to Div_cnt6b and Div45
#define SENSOR_BAYER_DEFAULT_PLL_PRE_DIV         (0x0002) // refer to R_PREDIV.
#define SENSOR_BAYER_DEFAULT_PLL_POS_DIV         (0x0001) // Keep as 1.
#define SENSOR_BAYER_DEFAULT_PLL_VT_PIX_DIV      (0x000a) // refer to R_DIVS, R_DIVL, R_SELD2P5, R_PREDIV and out_block_div
#define SENSOR_BAYER_DEFAULT_PLL_VT_SYS_DIV      (0x0001) // Keep as 1.

#define SENSOR_BAYER_DEFAULT_MAX_GAIN            (15.5) // reg 0x350a-0x350b, but seems only 0x350b is in use? Equation below. Min is 1x1x1x1, max is 2x2x2x1.9375=15.5
                                                        // ??? reg 0x3508-9 long gain
                                                        //
#define SENSOR_BAYER_DEFAULT_MIN_GAIN             (1.0) // reg 0x350b with all bits to be zero. Gain = (0x350b[6]+1) x (0x350b[5]+1) x (0x350b[4]+1) x (0x350b[3:0]/16+1)
                                                        // ??? reg 0x3a18-9 about AEC gain ceiling, what are they?
                                                        //

#define SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF     (6)     // Minmum margin between frame length and coarse integration time.

#define SENSOR_BAYER_DEFAULT_MAX_FRAME_LENGTH  (0xffff)  // spec claims register 0x380e-f is 12 bits
                                                         // but setting this to 0xffff actually increases
                                                         // max exposure time to a useful level. Possibly
                                                         // only the most significant 12 bits are used?
                                                         // use reg 0x3500 for LONG EXPO 1/3
                                                         // use reg 0x3501 for LONG EXPO 2/3
                                                         // use reg 0x3502 for LONG EXPO 3/3
                                                         // The max "coarse integration time" allowed in these registers are 0xffff
                                                         //
#define SENSOR_BAYER_DEFAULT_MAX_EXPOSURE_COARSE (SENSOR_BAYER_DEFAULT_MAX_FRAME_LENGTH-SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF)
                                                         // use reg 0x3500 for LONG EXPO 1/3
                                                         // use reg 0x3501 for LONG EXPO 2/3
                                                         // use reg 0x3502 for LONG EXPO 3/3
                                                         //
#define SENSOR_BAYER_DEFAULT_MIN_EXPOSURE_COARSE (0x002) //
                                                         // use reg 0x3500 for LONG EXPO 1/3
                                                         // use reg 0x3501 for LONG EXPO 2/3
                                                         // use reg 0x3502 for LONG EXPO 3/3, use 0x2 for 2 lines of integration time (register 0x3502  to be programmed as 0x2 << 4).

#define SENSOR_BAYER_DEFAULT_LINE_LENGTH_FULL     (0x0cb4)  // refer to {0x380c, 0x0c}
                                                            //          {0x380d, 0xb4}
#define SENSOR_BAYER_DEFAULT_FRAME_LENGTH_FULL    (0x07b0)  // refer to {0x380e, 0x07}
                                                            //          {0x380f, 0xb0}
#define SENSOR_BAYER_DEFAULT_EXPOSURE_COARSE_FULL   (SENSOR_BAYER_DEFAULT_FRAME_LENGTH_FULL-\
                                                     SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF)
                                                              // refer to {0x3500, 0x00}
                                                              // refer to {0x3501, 0xca}
                                                              // refer to {0x3502, 0xe0}
#define SENSOR_BAYER_DEFAULT_LINE_LENGTH_QUART       (0x08a8) // refer to {0x380c, 0x08}
                                                              // refer to {0x380d, 0xa8}
#define SENSOR_BAYER_DEFAULT_FRAME_LENGTH_QUART      (0x05a4) // refer to {0x380e, 0x05}
                                                              //          {0x380f, 0xa4}
#define SENSOR_BAYER_DEFAULT_EXPOSURE_COARSE_QUART   (SENSOR_BAYER_DEFAULT_FRAME_LENGTH_QUART-SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF)
                                                              // refer to {0x3500, 0x00}
                                                              // refer to {0x3501, 0x59}
                                                             // refer to {0x3502, 0xe0}
#define SENSOR_BAYER_DEFAULT_LINE_LENGTH_1080P_NEW       (0x0a84)
#define SENSOR_BAYER_DEFAULT_FRAME_LENGTH_1080P_NEW      (0x04a4)
#define SENSOR_BAYER_DEFAULT_EXPOSURE_COARSE_1080P_NEW   (SENSOR_BAYER_DEFAULT_FRAME_LENGTH_1080P_NEW-\
                                                         SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF)

#define SENSOR_BAYER_DEFAULT_LINE_LENGTH_1080P       (0x0a96)
#define SENSOR_BAYER_DEFAULT_FRAME_LENGTH_1080P      (0x049e)
#define SENSOR_BAYER_DEFAULT_EXPOSURE_COARSE_1080P   (SENSOR_BAYER_DEFAULT_FRAME_LENGTH_1080P-\
                                                      SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF)

#define SENSOR_BAYER_DEFAULT_LINE_LENGTH_QVGA        (0x0a04) // refer to {0x380c, 0x0a}
                                                              //          {0x380d, 0x04}
#define SENSOR_BAYER_DEFAULT_FRAME_LENGTH_QVGA       (0x0138) // refer to {0x380e, 0x01}
                                                              //          {0x380f, 0x38}
#define SENSOR_BAYER_DEFAULT_EXPOSURE_COARSE_QVGA   (SENSOR_BAYER_DEFAULT_LINE_LENGTH_QVGA-\
                                                     SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF)

#define DIFF_INTEGRATION_TIME_OF_ADDITION_MODE    (0.0f)
#define DIFF_INTEGRATION_TIME_OF_ELIMINATION_MODE (0.0f)
#define DIFF_INTEGRATION_TIME_OF_MODE            DIFF_INTEGRATION_TIME_OF_ELIMINATION_MODE


#define HACT_START_FULL (0)
#define VACT_START_FULL (2)
#define HACT_START_QUART (0)
#define VACT_START_QUART (2)
#define HACT_START_QVGA (0)
#define VACT_START_QVGA (2)

#if (BUILD_FOR_AOS == 1)
#define SUPPORT_1080P_RES 0
#define SUPPORT_QUARTER_RES 0
#define SUPPORT_QVGA_RES 0

#define MAX_OVERRIDE_LIST_SIZE (8)
#define HIBYTE(_x) (((_x) >> 8) & 0xFF)
#define LOBYTE(_x) (((_x)) & 0xFF)

#define LENS_FOCAL_LENGTH (4.76f)
#define LENS_HORIZONTAL_VIEW_ANGLE (60.4f)
#define LENS_VERTICAL_VIEW_ANGLE   (60.4f)
#else
#define SUPPORT_1080P_RES 1
#define SUPPORT_QUARTER_RES 1
#define SUPPORT_QVGA_RES 1
#endif

#define LENS_FOCAL_LENGTH (4.76f)
#define LENS_HORIZONTAL_VIEW_ANGLE (60.4f)
#define LENS_VERTICAL_VIEW_ANGLE   (60.4f)

// sensor mode that outputs frame of size
// 1920x1080.
#define NEW_1080P_SETTINGS 1

static const char *pOverrideFiles[] =
{
    "/data/camera_overrides.isp",
};

static const char *pFactoryBlobFiles[] =
{
    "/data/factory.bin",
    "/data/calibration.bin",
};

static NV_INLINE NvF32
SENSOR_GAIN_TO_F32(NvU16 x, NvF32 MaxGain, NvF32 MinGain)
{
    NvU16 reg0x350bBit6;
    NvU16 reg0x350bBit5;
    NvU16 reg0x350bBit4;
    NvU16 reg0x350bBit0To3;

    reg0x350bBit6 = (x>>6) & 0x1;
    reg0x350bBit5 = (x>>5) & 0x1;
    reg0x350bBit4 = (x>>4) & 0x1;
    reg0x350bBit0To3 = x & 0xff;
    return ((NvF32)reg0x350bBit6+1.f) * ((NvF32)reg0x350bBit5+1.f) * ((NvF32)reg0x350bBit5+1.f) * ((NvF32)reg0x350bBit0To3/16.f+1.f);
}

static NV_INLINE NvU16
SENSOR_F32_TO_GAIN(NvF32 x, NvF32 MaxGain, NvF32 MinGain)
{
    NvU16 reg0x350bBit6=0x0;
    NvU16 reg0x350bBit5=0x0;
    NvU16 reg0x350bBit4=0x0;
    NvU16 reg0x350bBit0To3=0x0;

    NvF32 gainForBit0To3;
    NvF32 gainReminder;

    // for OV5650, gain is up to x15.5
    // 15.5 (max), 7.75, 3.875, 1.9375, 1.0 (min)
    gainReminder = x;
    if(x > MaxGain/2.0)
    {
        reg0x350bBit6 = 0x1;
        reg0x350bBit5 = 0x1;
        reg0x350bBit4 = 0x1;
        gainReminder = (NvF32)(x / 8.0);
    }
    else if(x > MaxGain/4.0)
    {
        reg0x350bBit6 = 0x0;
        reg0x350bBit5 = 0x1;
        reg0x350bBit4 = 0x1;
        gainReminder = (NvF32)(x / 4.0);
    }
    else if(x > MaxGain/8.0)
    {
        reg0x350bBit6 = 0x0;
        reg0x350bBit5 = 0x0;
        reg0x350bBit4 = 0x1;
        gainReminder = (NvF32)(x / 2.0);
    }

    if(gainReminder > MaxGain/8.0)
    {
        gainReminder = (NvF32)(MaxGain/8.0);
    }
    if(gainReminder < MinGain)
    {
        gainReminder = MinGain;
    }

    gainForBit0To3 = (NvF32)((gainReminder-MinGain)*16.0);
    reg0x350bBit0To3 = (NvU16)(gainForBit0To3);
    if((gainForBit0To3 - (NvF32)reg0x350bBit0To3 > 0.5) && (reg0x350bBit0To3!=0xf))
    {
        reg0x350bBit0To3 += 1;
    }
    reg0x350bBit0To3 &= 0xf; // might not need this?

    NV_DEBUG_PRINTF(("x=%f, MaxGain=%f, reg0x350bBit6=%d, reg0x350bBit5=%d, reg0x350bBit4=%d, reg0x350bBit0To3=%d\n",
                     x, MaxGain, reg0x350bBit6, reg0x350bBit5, reg0x350bBit4, reg0x350bBit0To3));


    return (reg0x350bBit6<<6) | (reg0x350bBit5<<5) | (reg0x350bBit4<<4) | (reg0x350bBit0To3);
}

/**
 * ----------------------------------------------------------
 *  Start of Phase 0 code
 * ----------------------------------------------------------
 */

/**
 * Sensor Specific Variables/Defines. Phase 0.
 *
 * If a new sensor code is created from this template, the following
 * variable/defines should be created for the new sensor specifically.
 * 1. A sensor capabilities (NvOdmImagerCapabilities) for this specific sensor
 * 2. A sensor set mode sequence list  (an array of SensorSetModeSequence)
 *    consisting of pairs of a sensor mode and corresponding set mode sequence
 * 3. A set mode sequence for each mode defined in 2.
 */

/**
 * Sensor bayer's capabilities. Phase 0. Sensor Dependent.
 * This is the first thing the camera driver requests. The information here
 * is used to setup the proper interface code (CSI, VIP, pixel type), start
 * up the clocks at the proper speeds, and so forth.
 * For Phase 0, one could start with these below as an example for a bayer
 * sensor and just update clocks from what is given in the reference documents
 * for that device. The FAE may have specified the clocks when giving the
 * reccommended i2c settings.
 */
static NvOdmImagerCapabilities g_SensorBayerCaps =
{
    "OV5650",  // string identifier, used for debugging
    NvOdmImagerSensorInterface_SerialA,
    {
        NvOdmImagerPixelType_BayerBGGR,
    },
    NvOdmImagerOrientation_0_Degrees, // rotation with respect to main display
    NvOdmImagerDirection_Away,          // toward or away the main display
    6000, // initial sensor clock (in khz)
    // preferred clock profile: {mclk, ratio = max_output_pixelclock/mclk}
    // max_output_pixelclock should be >= max pixel clock in the mode table
    // for this sensor, it is mode 2080x1164_30: 2080 x 1164 x 30fps = 72.64MHz
    { {24000, 72.64f/24.f}},
    {
        NvOdmImagerSyncEdge_Rising, // hsync polarity
        NvOdmImagerSyncEdge_Rising, // vsync polarity
        NV_FALSE,                   // uses mclk on vgp0
    }, // parallel settings (VIP)
    {
        NVODMSENSORMIPITIMINGS_PORT_CSI_A,
        NVODMSENSORMIPITIMINGS_DATALANES_TWO,
        NVODMSENSORMIPITIMINGS_VIRTUALCHANNELID_ONE,
        1, // USE !CONTINUOUS_CLOCK,
        0xb
    }, // serial settings (CSI)
    { 16, 16 }, // min blank time, shouldn't need adjustment
    0, // preferred mode, which resolution to use on startup
#if (BUILD_FOR_VENTANA == 1)
    FOCUSER_NVC_GUID1,
#elif (BUILD_FOR_WHISTLER == 1)
    FOCUSER_AD5820_GUID,
#elif (BUILD_FOR_CARDHU == 1)
    FOCUSER_NVC_GUID1,
#else
    0, // only set if focuser device is available
#endif
#if (BUILD_FOR_CARDHU == 1)
    TORCH_NVC_GUID,
#elif (BUILD_FOR_VENTANA == 1)
    TORCH_NVC_GUID,
#else
    0, // only set if flash device is available
#endif
    NVODM_IMAGER_CAPABILITIES_END
};

// in case of AOS build, these sensor register programming should be in ODM code
// for honeycomb build, these should be in kernel
#if (BUILD_FOR_AOS == 1)
static DevCtrlReg16 SetModeSequence_2592X1944[] =
{
    {0x3008, 0x82},
    {0x3008, 0x42},
    {0x3103, 0x93},
    {0x3017, 0xff},
    {0x3018, 0xfc},
    {0x3706, 0x41},
    {0x3703, 0xe6},
    {0x3613, 0x44},
    {0x3630, 0x22},
    {0x3605, 0x4},
    {0x3606, 0x3f},
    {0x3712, 0x13},
    {0x370e, 0x0},
    {0x370b, 0x40},
    {0x3600, 0x50},
    {0x3601, 0xd},
    {0x3713, 0x2f},
    {0x3631, 0x22},
    {0x3612, 0x1a},
    {0x3604, 0x50},
    {0x3705, 0xda},
    {0x370a, 0x80},
    {0x370c, 0xa0},
    {0x3710, 0x28},
    {0x3702, 0x3a},
    {0x3704, 0x18},
    {0x3632, 0x5a},
    {0x3a18, 0x0},
    {0x3a19, 0xf8},
    {0x3a00, 0x38},
    {0x3800, 0x2},
    {0x3801, 0x58},
    {0x3803, 0xc},
    {0x380c, 0xc},
    {0x380d, 0xb4},
    {0x380e, 0x7},
    {0x380f, 0xb0},
    {0x3830, 0x50},
    {0x3804, 0x0a},
    {0x3805, 0x20},
    {0x3806, 0x07},
    {0x3807, 0xa0},
    {0x3808, 0x0A},
    {0x3809, 0x20},
    {0x380a, 0x07},
    {0x380b, 0xa0},
    {0x3a08, 0x12},
    {0x3a09, 0x70},
    {0x3a0a, 0xf},
    {0x3a0b, 0x60},
    {0x3a0d, 0x6},
    {0x3a0e, 0x6},
    {0x3815, 0x82},
    {0x5059, 0x80},
    {0x505a, 0xa},
    {0x505b, 0x2e},
    {0x3a1a, 0x6},
    {0x3503, 0x0},
    {0x3c01, 0x80},
    {0x401c, 0x48},
    {0x4000, 0x1},
    {0x401d, 0x28},
    {0x5046, 0x0},
    {0x3810, 0x0},
    {0x3836, 0x0},
    {0x5000, 0x0},
    {0x5001, 0x0},
    {0x5002, 0x0},
    {0x503d, 0x0},
    {0x5901, 0x0},
    {0x585a, 0x1},
    {0x585b, 0x2c},
    {0x585c, 0x1},
    {0x585d, 0x93},
    {0x585e, 0x1},
    {0x585f, 0x90},
    {0x5860, 0x1},
    {0x5861, 0xd},
    {0x5180, 0xc0},
    {0x5184, 0x0},
    {0x470a, 0x0},
    {0x470b, 0x0},
    {0x470c, 0x0},
    {0x300f, 0x8f},
    {0x3011, 0x14},
    {0x3603, 0xa7},
    {0x3615, 0x50},
    {0x3632, 0x55},
    {0x3620, 0x56},
    {0x3621, 0x2f},
    {0x381a, 0x3c},
    {0x3818, 0xc0},
    {0x3003, 0x03},
    {0x4801, 0x0f},
    {0x300e, 0x0c},
    {0x4803, 0x50},
    {0x4800, 0x14},
    {0x3003, 0x01},
    {0x3008, 0x2},
    {0x300f, 0x8F},

    {SEQUENCE_FAST_SETMODE_END, 0},
    {SEQUENCE_END, 0x0000}
};
#endif

typedef struct SensorBayerOV5650ModeDependentSettingsRec
{
    NvU32 LineLength;
    NvU32 FrameLength;
    NvU32 CoarseTime;
    NvU32 MinFrameLength;
    NvF32 InherentGain;
    NvBool SupportsBinningControl;
    NvBool BinningControlEnabled;
#if (BUILD_FOR_AOS == 1)
    NvBool SupportsFastMode;
#endif
} SensorBayerOV5650ModeDependentSettings;

SensorBayerOV5650ModeDependentSettings ModeDependentSettings_OV5650_2592X1944 =
{
    SENSOR_BAYER_DEFAULT_LINE_LENGTH_FULL,
    SENSOR_BAYER_DEFAULT_FRAME_LENGTH_FULL,
    SENSOR_BAYER_DEFAULT_EXPOSURE_COARSE_FULL,
    SENSOR_BAYER_DEFAULT_FRAME_LENGTH_FULL,
    1.0f,
    NV_FALSE,
    NV_FALSE,
#if (BUILD_FOR_AOS == 1)
    NV_TRUE, // support fast mode
#endif
};

SensorBayerOV5650ModeDependentSettings ModeDependentSettings_OV5650_1296X972 =
{
    SENSOR_BAYER_DEFAULT_LINE_LENGTH_QUART,
    SENSOR_BAYER_DEFAULT_FRAME_LENGTH_QUART,
    SENSOR_BAYER_DEFAULT_EXPOSURE_COARSE_QUART,
    SENSOR_BAYER_DEFAULT_FRAME_LENGTH_QUART,
    2.0f,
    NV_TRUE,
    NV_TRUE,
#if (BUILD_FOR_AOS == 1)
    NV_TRUE, // support fast mode
#endif
};

SensorBayerOV5650ModeDependentSettings ModeDependentSettings_OV5650_1264X704 =
{
    0x0790, // LINE_LENGTH
    0x03f4, // FRAME_LENGTH
    0x03ec, // EXPOSURE_COARSE
    0x03f4, // FRAME_LENGTH
    2.0f, // Binning does summation in Horizontal direction
    NV_TRUE,
    NV_TRUE,
#if (BUILD_FOR_AOS == 1)
    NV_FALSE, // support fast mode
#endif
};

SensorBayerOV5650ModeDependentSettings ModeDependentSettings_OV5650_2080x1164 =
{
    SENSOR_BAYER_DEFAULT_LINE_LENGTH_1080P,
    SENSOR_BAYER_DEFAULT_FRAME_LENGTH_1080P,
    SENSOR_BAYER_DEFAULT_EXPOSURE_COARSE_1080P,
    SENSOR_BAYER_DEFAULT_FRAME_LENGTH_1080P,
    1.0f,
    NV_FALSE,
    NV_FALSE,
#if (BUILD_FOR_AOS == 1)
    NV_TRUE, // support fast mode
#endif
};

SensorBayerOV5650ModeDependentSettings ModeDependentSettings_OV5650_1920x1080 =
{
    SENSOR_BAYER_DEFAULT_LINE_LENGTH_1080P_NEW,
    SENSOR_BAYER_DEFAULT_FRAME_LENGTH_1080P_NEW,
    SENSOR_BAYER_DEFAULT_EXPOSURE_COARSE_1080P_NEW,
    SENSOR_BAYER_DEFAULT_FRAME_LENGTH_1080P_NEW,
    1.0f,
    NV_FALSE,
    NV_FALSE,
#if (BUILD_FOR_AOS == 1)
    NV_TRUE, // support fast mode
#endif
};

SensorBayerOV5650ModeDependentSettings ModeDependentSettings_OV5650_320X240 =
{
    SENSOR_BAYER_DEFAULT_LINE_LENGTH_QVGA,
    SENSOR_BAYER_DEFAULT_FRAME_LENGTH_QVGA,
    SENSOR_BAYER_DEFAULT_EXPOSURE_COARSE_QVGA,
    SENSOR_BAYER_DEFAULT_FRAME_LENGTH_QVGA,
    1.0f,
    NV_FALSE,
    NV_FALSE,
#if (BUILD_FOR_AOS == 1)
    NV_FALSE, // support fast mode
#endif
};

/**
 * A list of sensor mode and its SetModeSequence. Phase 0. Sensor Dependent.
 * When SetMode is called, it searches the list for desired resolution and
 * frame rate and write the SetModeSequence through i2c.
 * For Phase 3, more resolutions will be added to this table, allowing
 * for peak performance during Preview, D1 and 720P encode.
 */
static SensorSetModeSequence *g_pSensorBayerSetModeSequenceList;

/*
   AW: Note that there is no set sequence data, as the actual mode set is done
   in the ov5650 kernel driver.
*/
#if (BUILD_FOR_AOS == 0)
static SensorSetModeSequence g_SensorBayerSetModeSequenceList[] =
{
     // WxH         x,y    fps    set mode sequence
     // pll_multiplier (pll multiplier should come from sensor vendor.  MCLK*pll_multiplier = sensor_output_clk (MIPI_CLK or PCLK)
    {{{2592, 1944}, {HACT_START_FULL,  VACT_START_FULL}, 14, 1.0, 20.0, NvOdmImagerNoCropMode,
                {{600, 12, 600 + 2592, 12 + 1952}, {1.f, 1.f}},NvOdmImagerModeType_Normal,},
                NULL, &ModeDependentSettings_OV5650_2592X1944}, // Full Mode, sensor is outputing 2592X1944.
#if SUPPORT_QUARTER_RES
    {{{1296, 972},  {HACT_START_QUART, VACT_START_QUART}, 30, 1.0, 20.0, NvOdmImagerNoCropMode,
                {{0, 0, 0, 0}, {2.f, 2.f}},NvOdmImagerModeType_Normal,},
                NULL, &ModeDependentSettings_OV5650_1296X972}, // preview/encoding mode, same aspect ratio as above one.
#endif
#if SUPPORT_1080P_RES
#if NEW_1080P_SETTINGS
    {{{1920, 1080},  {0, 0}, 30, 1.0, 20.0, NvOdmImagerPartialCropMode,
                {{0, 0, 0, 0}, {0.f, 0.f}}, NvOdmImagerModeType_Normal,},
                NULL, &ModeDependentSettings_OV5650_1920x1080},
#else
    {{{2080, 1164},  {HACT_START_QUART, VACT_START_QUART}, 30, 1.0, 20.0, NvOdmImagerPartialCropMode,
                {{0, 0, 0, 0}, {1.f, 1.f}}, NvOdmImagerModeType_Normal,},
                NULL, &ModeDependentSettings_OV5650_2080x1164},
#endif
#endif
#if SUPPORT_QVGA_RES
    {{{320, 240},  {HACT_START_QVGA, VACT_START_QVGA}, 120, 1.0, 20.0, NvOdmImagerPartialCropMode,
                {{0, 0, 0, 0}, {2.f, 2.f}}, NvOdmImagerModeType_Normal,},
                NULL, &ModeDependentSettings_OV5650_320X240},
#endif
};

#else
static SensorSetModeSequence g_SensorBayerSetModeSequenceList[] =
{
     // WxH         x,y    fps    set mode sequence
    {{{2592, 1942}, {HACT_START_FULL,  VACT_START_FULL},  14, 1.0}, SetModeSequence_2592X1944, &ModeDependentSettings_OV5650_2592X1944}, // Full Mode, sensor is outputing 2592X1944.
#if SUPPORT_QUARTER_RES
    {{{1296, 972},  {HACT_START_QUART, VACT_START_QUART}, 30, 1.0}, SetModeSequence_1296X972, &ModeDependentSettings_OV5650_1296X972}, // preview/encoding mode, same aspect ratio as above one.
#endif
#if SUPPORT_1080P_RES
    {{{1920, 1088},  {HACT_START_QUART, VACT_START_QUART}, 30, 1.0}, NULL, &ModeDependentSettings_OV5650_1920x1088},
#endif
};
#endif

#if (BUILD_FOR_AOS == 0)
static SensorSetModeSequence g_SensorBayerSetModeSequenceStereoList[] =
{
     // WxH         x,y    fps    set mode sequence
     // pll_multiplier (pll multiplier should come from sensor vendor.  MCLK*pll_multiplier = sensor_output_clk (MIPI_CLK or PCLK)
    {{{2592, 1944}, {HACT_START_FULL,  VACT_START_FULL},  14, 1.0, 20.0, NvOdmImagerNoCropMode,
                {{0, 0, 0, 0}, {1.f, 1.f}}, NvOdmImagerModeType_Normal,},
                NULL, &ModeDependentSettings_OV5650_2592X1944}, // Full Mode, sensor is outputing 2592X1944.
#if 0 //Bug# 941425
    {{{1296, 972},  {HACT_START_QUART, VACT_START_QUART}, 30, 1.0, 20.0, NvOdmImagerNoCropMode, NvOdmImagerModeType_Normal},
                NULL, &ModeDependentSettings_OV5650_1296X972}, // preview/encoding mode, same aspect ratio as above one.
#endif
#if SUPPORT_1080P_RES
    {{{2080, 1164},  {HACT_START_QUART, VACT_START_QUART}, 30, 1.0, 20.0, NvOdmImagerPartialCropMode,
                {{0, 0, 0, 0}, {1.f, 1.f}}, NvOdmImagerModeType_Normal,},
                NULL, &ModeDependentSettings_OV5650_2080x1164},
#endif
};

#else
static SensorSetModeSequence g_SensorBayerSetModeSequenceStereoList[] =
{
     // WxH         x,y    fps    set mode sequence
    {{{2592, 1942}, {HACT_START_FULL,  VACT_START_FULL},  14, 1.0}, SetModeSequence_2592X1944, &ModeDependentSettings_OV5650_2592X1944}, // Full Mode, sensor is outputing 2592X1944.
#if SUPPORT_QUARTER_RES
    {{{1296, 972},  {HACT_START_QUART, VACT_START_QUART}, 30, 1.0}, SetModeSequence_1296X972, &ModeDependentSettings_OV5650_1296X972}, // preview/encoding mode, same aspect ratio as above one.
#endif
#if SUPPORT_1080P_RES
    {{{1920, 1088},  {HACT_START_QUART, VACT_START_QUART}, 30, 1.0}, NULL, &ModeDependentSettings_OV5650_1920x1088},
#endif
};
#endif

/**
 * ----------------------------------------------------------
 *  Start of Phase 1, Phase 2, and Phase 3 code
 * ----------------------------------------------------------
 */

/**
 * Sensor bayer's private context. Phase 1.
 */
typedef struct SensorBayerContextRec
{
#if (BUILD_FOR_AOS == 0)
    // Phase 1 variables.
    int   camera_fd;
#else
    // Phase 1 variables.
    NvOdmImagerI2cConnection I2c;
    NvOdmImagerI2cConnection I2cR;
    ImagerGpioConfig GpioConfig;
    NvBool FastSetMode;

#endif

    NvU32 ModeIndex;
    NvU32 NumModes;
    NvOdmImagerPowerLevel PowerLevel;

    // Phase 2 variables.
    NvBool SensorInitialized;

    NvU32 SensorInputClockkHz; // mclk (extclk)
    NvF32 SensorClockRatio;    // pclk/mclk

    NvF32 Exposure;
    NvF32 MaxExposure;
    NvF32 MinExposure;

    NvU32 FrameErrorCount;

    NvF32 Gains[4];
    NvF32 MaxGain;
    NvF32 MinGain;
    NvF32 InherentGain;
    NvBool SupportsBinningControl;
    NvBool BinningControlEnabled;
    NvBool BinningControlDirty;

    NvF32 FrameRate;
    NvF32 MaxFrameRate;
    NvF32 MinFrameRate;
    NvF32 RequestedMaxFrameRate;

    // Phase 2 variables. Sensor Dependent.
    // These are used to set or get exposure, frame rate, and so on.
    NvU32 PllMult;
    NvF32 PllPreDiv;
    NvU32 PllPosDiv;
    NvU32 PllVtPixDiv;
    NvU32 PllVtSysDiv;

    NvU32 CoarseTime;
    NvU32 VtPixClkFreqHz;

    NvU32 LineLength;
    NvU32 FrameLength;

    NvU32 MaxFrameLength;
    NvU32 MinFrameLength;
    NvBool TestPatternMode;

    NvBool showSkipWriteGains;
    NvBool showSkipWriteExposure;

    NvOdmImagerStereoCameraMode StereoCameraMode;

    NvOdmImagerTestPattern TestPattern;
}SensorBayerContext;

typedef const char *CString;

/**
 * Static functions
 * Some compilers require these to be declared upfront.
 */
static NvBool SensorBayer_Open(NvOdmImagerHandle hImager);

static void SensorBayer_Close(NvOdmImagerHandle hImager);

static void
SensorBayer_GetCapabilities(
    NvOdmImagerHandle hImager,
    NvOdmImagerCapabilities *pCapabilities);

static void
SensorBayer_ListModes(
    NvOdmImagerHandle hImager,
    NvOdmImagerSensorMode *pModes,
    NvS32 *pNumberOfModes);

static NvBool
SensorBayer_SetMode(
    NvOdmImagerHandle hImager,
    const SetModeParameters *pParameters,
    NvOdmImagerSensorMode *pSelectedMode,
    SetModeParameters *pResult);

static NvBool
SensorBayer_SetPowerLevel(
    NvOdmImagerHandle hImager,
    NvOdmImagerPowerLevel PowerLevel);

static NvBool
SensorBayer_SetParameter(
    NvOdmImagerHandle hImager,
    NvOdmImagerParameter Param,
    NvS32 SizeOfValue,
    const void *pValue);

static NvBool
SensorBayer_GetParameter(
    NvOdmImagerHandle hImager,
    NvOdmImagerParameter Param,
    NvS32 SizeOfValue,
    void *pValue);

static void
SensorBayer_GetPowerLevel(
    NvOdmImagerHandle hImager,
    NvOdmImagerPowerLevel *pPowerLevel);

static NV_INLINE NvBool
SensorOV5650_ChooseModeIndex(
    SensorBayerContext *pContext,
    NvSize Resolution,
    NvU32 *pIndex)
{
    NvU32 Index;

    for (Index = 0; Index < pContext->NumModes; Index++)
    {
        if ((Resolution.width ==
             g_pSensorBayerSetModeSequenceList[Index].Mode.
             ActiveDimensions.width) &&
            (Resolution.height ==
             g_pSensorBayerSetModeSequenceList[Index].Mode.
             ActiveDimensions.height))
        {
            *pIndex = Index;
            return NV_TRUE;
        }
    }

    return NV_FALSE;
}

/**
 * SensorBayer_WriteExposure. Phase 2. Sensor Dependent.
 * Calculate and write the values of the sensor registers for the new
 * exposure time.
 * Without this, calibration will not be able to capture images
 * at various brightness levels, and the final product won't be able
 * to compensate for bright or dark scenes.
 *
 * if pFrameLength or pCoarseTime is not NULL, return the resulting
 * frame length or corase integration time instead of writing the
 * register.
 */

#if (BUILD_FOR_AOS == 0)

static NvBool
SensorBayer_WriteExposure(
    SensorBayerContext *pContext,
    const NvF32 *pNewExposureTime,
    NvU32 *pFrameLength,
    NvU32 *pCoarseTime)
{
    NvF32 Freq = (NvF32)pContext->VtPixClkFreqHz;
    NvF32 LineLength = (NvF32)pContext->LineLength;
    NvF32 ExpTime = *pNewExposureTime;
    NvU32 NewCoarseTime = 0;
    NvU32 NewFrameLength = 0;

    if (pContext->TestPatternMode)
        return NV_FALSE;

    if (*pNewExposureTime > pContext->MaxExposure)
    {
        NV_DEBUG_PRINTF(("WARNING: ExpTime (%f) > MaxExposure (%f) Using MaxExposure\n",
                        *pNewExposureTime, pContext->MaxExposure));
        ExpTime = pContext->MaxExposure;
    }
    if (*pNewExposureTime < pContext->MinExposure)
    {
        NV_DEBUG_PRINTF(("WARNING: ExpTime (%f) < MinExposure (%f) Using MinExposure\n",
                        *pNewExposureTime, pContext->MinExposure));
        ExpTime = pContext->MinExposure;
    }
    // Here, we have to decide if the new exposure time is valid
    // based on the sensor and current sensor settings.
    // Within smaller size mode, 0.23 should be changed to 0.11 if using V-addition calculation
    NewCoarseTime = (NvU32)((Freq * ExpTime) / LineLength+DIFF_INTEGRATION_TIME_OF_MODE);
    if( NewCoarseTime == 0 ) NewCoarseTime = 1;
    NewFrameLength = NewCoarseTime + SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF;

    if (pContext->RequestedMaxFrameRate > 0.0f)
    {
        NvU32 RequestedMinFrameLengthLines = 0;
        RequestedMinFrameLengthLines =
           (NvU32)(Freq / (LineLength * pContext->RequestedMaxFrameRate));

        if (NewFrameLength < RequestedMinFrameLengthLines)
            NewFrameLength = RequestedMinFrameLengthLines;
    }

    // Clamp to sensor's limit
    if (NewFrameLength > pContext->MaxFrameLength)
        NewFrameLength = pContext->MaxFrameLength;
    else if (NewFrameLength < pContext->MinFrameLength)
        NewFrameLength = pContext->MinFrameLength;

    // return the new frame length
    if (pFrameLength)
    {
        *pFrameLength = NewFrameLength;
    }

    if (NewFrameLength != pContext->FrameLength)
    {
        // write new value only when pFrameLength is NULL
        if (!pFrameLength)
        {
            int ret;
            ret = ioctl(pContext->camera_fd, OV5650_IOCTL_SET_FRAME_LENGTH,
                        NewFrameLength);
            if (ret < 0)
                NvOsDebugPrintf("ioctl to set mode failed %s\n", strerror(errno));
        }

        // Update variables depending on FrameLength.
        pContext->FrameLength = NewFrameLength;
        pContext->FrameRate = (NvF32)pContext->VtPixClkFreqHz /
                              (NvF32)(pContext->FrameLength * pContext->LineLength);
    }
    else
    {
        NV_DEBUG_PRINTF(("Frame length keep as before : %d \n", NewFrameLength));
    }

    // FrameLength should have been updated but we have to check again
    // in case FrameLength gets clamped.
    if (NewCoarseTime >= pContext->FrameLength - SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF)
    {
        NewCoarseTime = pContext->FrameLength - SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF;
    }

    // return the new coarse time
    if (pCoarseTime)
    {
        *pCoarseTime = NewCoarseTime;
    }

    if (pContext->CoarseTime != NewCoarseTime)
    {
        // write new coarse time only when pCoarseTime is NULL
        if (!pCoarseTime)
        {
            int ret;
            ret = ioctl(pContext->camera_fd, OV5650_IOCTL_SET_COARSE_TIME,
                        NewCoarseTime);
            if (ret < 0)
                NvOsDebugPrintf("ioctl to set mode failed %s\n", strerror(errno));
        }

        // Calculate the new exposure based on the sensor and sensor settings.
        pContext->Exposure = ((NewCoarseTime -
                               (NvF32)DIFF_INTEGRATION_TIME_OF_MODE) *
                              (NvF32)LineLength) / Freq;
        pContext->CoarseTime = NewCoarseTime;
    }

    if (pContext->BinningControlDirty)
    {
        int ret = ioctl(pContext->camera_fd, OV5650_IOCTL_SET_BINNING, pContext->BinningControlEnabled);
        if (ret < 0)
        {
            NV_DEBUG_PRINTF(("ioctl to set binning mode failed %s", strerror(errno)));
        }
        else
        {
            pContext->InherentGain = pContext->BinningControlEnabled ? 2.0f : 1.0f;
            pContext->BinningControlDirty = NV_FALSE;
        }
    }

    return NV_TRUE;
}

#else

static NvBool
SensorBayer_WriteExposure(
    SensorBayerContext *pContext,
    const NvF32 *pNewExposureTime,
    DevCtrlReg16 **ppRegList)
{
#if 0
    if(pContext->showSkipWriteExposure == NV_TRUE)
    {
        NvOsDebugPrintf("Skipping write exposure\n");
        pContext->showSkipWriteExposure = NV_FALSE;
    }
    return NV_FALSE;
#else

    NvF32 Freq = (NvF32)pContext->VtPixClkFreqHz;
    NvF32 LineLength = (NvF32)pContext->LineLength;
    NvF32 ExpTime = *pNewExposureTime;
    NvU32 NewCoarseTime = 0;
    NvU32 NewFrameLength = 0;
    NvBool UpdateFrameLength = NV_FALSE;
    NvBool UpdateCoarseTime = NV_FALSE;

    // Here, we have to decide if the new exposure time is valid
    // based on the sensor and current sensor settings.
    //
    //  lines = exp / line_period
    //        = exp / (width / clock)
    //        = exp * clock / width
    NewCoarseTime = (NvU32)((Freq * ExpTime) / LineLength);
    if( NewCoarseTime == 0 ) NewCoarseTime = 1;
    NewFrameLength = NewCoarseTime + SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF;

    if (pContext->RequestedMaxFrameRate > 0.0f)
    {
        NvU32 RequestedMinFrameLengthLines = 0;
        RequestedMinFrameLengthLines =
           (NvU32)(Freq / (LineLength * pContext->RequestedMaxFrameRate));

        if (NewFrameLength < RequestedMinFrameLengthLines)
            NewFrameLength = RequestedMinFrameLengthLines;
    }

    // Clamp to sensor's limit
    if (NewFrameLength > pContext->MaxFrameLength)
        NewFrameLength = pContext->MaxFrameLength;
    else if (NewFrameLength < pContext->MinFrameLength)
        NewFrameLength = pContext->MinFrameLength;

    if(pContext->FrameLength != NewFrameLength)
        UpdateFrameLength = NV_TRUE;

    // check the CoarseTime again in case FrameLength gets clamped.
    if (NewCoarseTime >= NewFrameLength - SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF)
    {
        NewCoarseTime = NewFrameLength - SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF;
    }

    if(pContext->CoarseTime != NewCoarseTime)
        UpdateCoarseTime = NV_TRUE;

    // Current CoarseTime/FrameLength needs to be updated?
    if (UpdateFrameLength || UpdateCoarseTime)
    {
        if (ppRegList && *ppRegList)
        {
            if(UpdateFrameLength)
            {
                (*ppRegList)->RegAddr = 0x380e;
                (*ppRegList)->RegValue = (NewFrameLength >> 8) & 0xFF;
                (*ppRegList)++;
                (*ppRegList)->RegAddr = 0x380f;
                (*ppRegList)->RegValue = NewFrameLength & 0xFF;
                (*ppRegList)++;
            }
            if(UpdateCoarseTime)
            {
                (*ppRegList)->RegAddr = 0x3500;
                (*ppRegList)->RegValue = (NewCoarseTime >> 12) & 0xFF;
                (*ppRegList)++;
                (*ppRegList)->RegAddr = 0x3501;
                (*ppRegList)->RegValue = (NewCoarseTime >> 4)  & 0xFF;
                (*ppRegList)++;
                (*ppRegList)->RegAddr = 0x3502;
                (*ppRegList)->RegValue = ((NewCoarseTime)& 0xF)<<4;
                (*ppRegList)++;
            }
        }
        else
        {
            // to simplify, make coarse integration time as multiple of integer
            // number of lines, not multiple of 1/16 lines.

            // SRM_GROUP_ACCESS (group hold begin)
            NVODM_WRITE_RETURN_ON_ERROR(&pContext->I2c, 0x3212, 0x01);
            if(UpdateFrameLength)
            {
                NVODM_WRITE_RETURN_ON_ERROR(&pContext->I2c,
                                            0x380e, (NewFrameLength >> 8) &
                                            0xFF);
                NVODM_WRITE_RETURN_ON_ERROR(&pContext->I2c,
                                            0x380f, NewFrameLength & 0xFF);
            }
            if(UpdateCoarseTime)
            {
                // LONG EXPO 1/3
                NVODM_WRITE_RETURN_ON_ERROR(&pContext->I2c,
                                            0x3500, (NewCoarseTime >> 12) & 0xFF);
                // LONG EXPO 2/3
                NVODM_WRITE_RETURN_ON_ERROR(&pContext->I2c,
                                            0x3501, (NewCoarseTime >> 4)  & 0xFF);
                // LONG EXPO 3/3
                NVODM_WRITE_RETURN_ON_ERROR(&pContext->I2c,
                                            0x3502, ((NewCoarseTime)& 0xF)<<4);

            }
            // SRM_GROUP_ACCESS (group hold end)
            NVODM_WRITE_RETURN_ON_ERROR(&pContext->I2c, 0x3212, 0x11);
            // SRM_GROUP_ACCESS (group hold launch)
            NVODM_WRITE_RETURN_ON_ERROR(&pContext->I2c, 0x3212, 0xa1);

            if(UpdateFrameLength)
            {
                NV_DEBUG_PRINTF(("\n// ===== Frame length i2c update\n"));
                NV_DEBUG_PRINTF(("wri2c 0x%x 0x38 0x0e 0x%x \n",
                                pContext->I2c.DeviceAddr, (NewFrameLength >> 8) & 0xFF));
                NV_DEBUG_PRINTF(("wri2c 0x%x 0x38 0x0f 0x%x \n\n",
                                pContext->I2c.DeviceAddr, NewFrameLength & 0xFF));
            }
            if(UpdateCoarseTime)
            {
                NV_DEBUG_PRINTF(("\n// ==== Coarse integration time i2c update\n"));
                NV_DEBUG_PRINTF(("wri2c 0x%x 0x35 0x01 0x%x \n",
                                pContext->I2c.DeviceAddr, (NewCoarseTime>> 12) & 0xFF));
                NV_DEBUG_PRINTF(("wri2c 0x%x 0x35 0x02 0x%x \n",
                                pContext->I2c.DeviceAddr, (NewCoarseTime>> 4) & 0xFF));
                NV_DEBUG_PRINTF(("wri2c 0x%x 0x35 0x03 0x%x \n\n",
                                pContext->I2c.DeviceAddr, (NewCoarseTime & 0xF)<<4));
            }

            // Calculate the new exposure based on the sensor and sensor settings.
            pContext->Exposure = ((NewCoarseTime) * (NvF32)LineLength) / Freq;
            pContext->CoarseTime = NewCoarseTime;
            // Update the frame length
            pContext->FrameLength = NewFrameLength;
            // Update variables depending on FrameLength.
            pContext->FrameRate = (NvF32)pContext->VtPixClkFreqHz /
                                  (NvF32)(pContext->FrameLength * pContext->LineLength);
         }
    }

    NV_DEBUG_PRINTF(("new exposure = %f (%d lines)\n", pContext->Exposure, pContext->CoarseTime));
    NV_DEBUG_PRINTF(("Freq = %f    FrameLen=0x%x(%d), NewCoarseTime=0x%x(%d), LineLength=0x%x(%d)\n",
                    Freq,pContext->FrameLength,pContext->FrameLength,NewCoarseTime,NewCoarseTime,
                    pContext->LineLength,pContext->LineLength));
    NV_DEBUG_PRINTF(("new exposure = %f\n", pContext->Exposure));
#endif
    return NV_TRUE;
}
#endif

/**
 * SensorBayer_SetInputClock. Phase 2. Sensor Dependent.
 * Setting the input clock and updating the variables based on the input clock
 * The frame rate and exposure calculations are directly related to the
 * clock speed, so this is how the camera driver (the controller of the clocks)
 * can inform the nvodm driver of the current clock speed.
 */
static NvBool
SensorBayer_SetInputClock(
    SensorBayerContext *pContext,
    const NvOdmImagerClockProfile *pClockProfile)
{

    if (pClockProfile->ExternalClockKHz <
        g_SensorBayerCaps.InitialSensorClockRateKHz)
        return NV_FALSE;

    pContext->SensorInputClockkHz = pClockProfile->ExternalClockKHz;
    pContext->SensorClockRatio = pClockProfile->ClockMultiplier;

    // if sensor is initialized (SetMode has been called), then we need to
    // update necessary sensor variables (based on this particular sensor)
    if (pContext->SensorInitialized)
    {
        pContext->VtPixClkFreqHz =
            (NvU32)((pContext->SensorInputClockkHz * 1000 * pContext->PllMult) /
                    (pContext->PllPreDiv * pContext->PllVtPixDiv *
                     pContext->PllVtSysDiv));
        pContext->Exposure =
                  (((NvF32)pContext->CoarseTime - DIFF_INTEGRATION_TIME_OF_MODE) *
                    (NvF32)pContext->LineLength) /(NvF32)pContext->VtPixClkFreqHz;
        pContext->MaxExposure =
                  (((NvF32)SENSOR_BAYER_DEFAULT_MAX_EXPOSURE_COARSE - DIFF_INTEGRATION_TIME_OF_MODE) *
                    (NvF32)pContext->LineLength ) / (NvF32)pContext->VtPixClkFreqHz;
        pContext->MinExposure =
                  (((NvF32)SENSOR_BAYER_DEFAULT_MIN_EXPOSURE_COARSE - DIFF_INTEGRATION_TIME_OF_MODE) *
                    (NvF32)pContext->LineLength ) / (NvF32)pContext->VtPixClkFreqHz;

    }
    return NV_TRUE;
}


/**
 * SensorBayer_WriteGains. Phase 2. Sensor Dependent.
 * Writing the sensor registers for the new gains.
 * Just like exposure, this allows the sensor to brighten an image. If the
 * exposure time is insufficient to make the picture viewable, gains are
 * applied.  During calibration, the gains will be measured for maximum
 * effectiveness before the noise level becomes too high.
 * If per-channel gains are available, they are used to normalize the color.
 * Most sensors output overly greenish images, so the red and blue channels
 * are increased.
 *
 * if pGain is not NULL, return the resulting gain instead of writing the
 * register.
 */

#if (BUILD_FOR_AOS == 0)
static NvBool
SensorBayer_WriteGains(
    SensorBayerContext *pContext,
    const NvF32 *pGains,
    NvU16 *pGain)
{
// OV5650's
// manual gain control, reg 0x350b, gain formula is
//
//   Gain = (0x350b[6]+1) x (0x350b[5]+1) x (0x350b[4]+1) x (0x350b[3:0]/16+1)
//

    NvU32 i = 1;
    NvU16 NewGains = 0;

    if (pGains[i] > pContext->MaxGain)
        return NV_FALSE;
    if (pGains[i] < pContext->MinGain)
        return NV_FALSE;

    // prepare and write register 0x350b
    NewGains = SENSOR_F32_TO_GAIN(pGains[i], pContext->MaxGain, pContext->MinGain);

    if (pGain)
    {
        *pGain = NewGains;
    }
    else
    {
        int ret;
        ret = ioctl(pContext->camera_fd, OV5650_IOCTL_SET_GAIN, NewGains);
        if (ret < 0)
            NvOsDebugPrintf("ioctl to set gain failed %s\n", strerror(errno));
    }

    NvOdmOsMemcpy(pContext->Gains, pGains, sizeof(NvF32)*4);

    NV_DEBUG_PRINTF(("new gains = %f, %f, %f, %f\n\n", pGains[0],
        pGains[1], pGains[2], pGains[3]));

    return NV_TRUE;
}
#else
static NvBool
SensorBayer_WriteGains(
    SensorBayerContext *pContext,
    const NvF32 *pGains,
    DevCtrlReg16 **ppRegList)
{
// OV5650's
// manual gain control, reg 0x350b, gain formula is
//
//   Gain = (0x350b[6]+1) x (0x350b[5]+1) x (0x350b[4]+1) x (0x350b[3:0]/16+1)
//
#if 0
    if(pContext->showSkipWriteGains == NV_TRUE)
    {
        NvOsDebugPrintf("Skipping write gains\n");
        pContext->showSkipWriteGains = NV_FALSE;
    }
    return NV_FALSE;
#else

    NvU16 NewGains = 0;
    // This sensor does not support per-channel gains, so simply pick
    // the first green gain and use that.
    NvF32 ReqGains = pGains[1];

    // Check against limits
    if (ReqGains > pContext->MaxGain)
        return NV_FALSE;
    if (ReqGains < pContext->MinGain)
        return NV_FALSE;

    // prepare and write register 0x350b
    NewGains = SENSOR_F32_TO_GAIN(ReqGains, pContext->MaxGain, pContext->MinGain);

    if (ppRegList && *ppRegList)
    {
        (*ppRegList)->RegAddr = 0x350b;
        (*ppRegList)->RegValue = NewGains;
        (*ppRegList)++;
    }
    else
    {
        NVODM_WRITE_RETURN_ON_ERROR(&pContext->I2c, 0x350b, NewGains);
    }


    NV_DEBUG_PRINTF(("// ======================== gain i2c update\n"));
    NV_DEBUG_PRINTF(("wri2c 0x%x 0x35 0x0b 0x%x \n",
                    pContext->I2c.DeviceAddr, NewGains));

    // Remember the originally requested gains.
    NvOdmOsMemcpy(pContext->Gains, pGains, sizeof(NvF32)*4);

    NV_DEBUG_PRINTF(("new gains = %f (actually programmed = %f)\n",
        pGains[1], ReqGains));


#endif

    return NV_TRUE;
}
#endif

#if (BUILD_FOR_AOS == 0)
static NvBool
SensorBayer_GroupHold(
    SensorBayerContext *pContext,
    const NvOdmImagerSensorAE *sensorAE)
{
    int ret;
    struct ov5650_ae ae;
    NvU32 i = 1;

    NvF32 Freq = (NvF32)pContext->VtPixClkFreqHz;
    NvF32 LineLength = (NvF32)pContext->LineLength;
    NvF32 ExpTime = sensorAE->ET;
    NvU32 NewCoarseTime = 0;
    NvU32 NewFrameLength = 0;

    NvOdmOsMemset(&ae, 0, sizeof(struct ov5650_ae));

    if (sensorAE->gains_enable==NV_TRUE) {
        if (sensorAE->gains[i] > pContext->MaxGain)
            return NV_FALSE;
        if (sensorAE->gains[i] < pContext->MinGain)
            return NV_FALSE;

        // prepare and write register 0x350b
        ae.gain = SENSOR_F32_TO_GAIN(sensorAE->gains[i], pContext->MaxGain, pContext->MinGain);
        ae.gain_enable = NV_TRUE;
        NvOdmOsMemcpy(pContext->Gains, sensorAE->gains, sizeof(NvF32)*4);
    }

    if (sensorAE->ET_enable==NV_TRUE) {

        if (ExpTime > pContext->MaxExposure ||
            ExpTime < pContext->MinExposure)
        {
            NV_DEBUG_PRINTF(("new exptime over limit! New = %f out of (%f, %f)\n",
                             ExpTime,
                             pContext->MaxExposure,
                             pContext->MinExposure));
            return NV_FALSE;
        }

        // Here, we have to decide if the new exposure time is valid
        // based on the sensor and current sensor settings.
        // Within smaller size mode, 0.23 should be changed to 0.11 if using V-addition calculation
        NewCoarseTime = (NvU32)((Freq * ExpTime) / LineLength+DIFF_INTEGRATION_TIME_OF_MODE);
        if( NewCoarseTime == 0 )
            NewCoarseTime = 1;
        NewFrameLength = NewCoarseTime + SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF;

        if (pContext->RequestedMaxFrameRate > 0.0f)
        {
            NvU32 RequestedMinFrameLengthLines = 0;
            RequestedMinFrameLengthLines =
               (NvU32)(Freq / (LineLength * pContext->RequestedMaxFrameRate));
            if (NewFrameLength < RequestedMinFrameLengthLines)
                NewFrameLength = RequestedMinFrameLengthLines;
        }

        // Clamp to sensor's limit
        if (NewFrameLength > pContext->MaxFrameLength)
            NewFrameLength = pContext->MaxFrameLength;
        else if (NewFrameLength < pContext->MinFrameLength)
            NewFrameLength = pContext->MinFrameLength;

        if (NewFrameLength != pContext->FrameLength)
        {
            ae.frame_length = NewFrameLength;
            ae.frame_length_enable = NV_TRUE;

            // Update variables depending on FrameLength.
            pContext->FrameLength = NewFrameLength;
            pContext->FrameRate = (NvF32)pContext->VtPixClkFreqHz /
                              (NvF32)(pContext->FrameLength * pContext->LineLength);
        }
        else
        {
            NV_DEBUG_PRINTF(("Frame length keep as before : %d \n", NewFrameLength));
        }

        // FrameLength should have been updated but we have to check again
        // in case FrameLength gets clamped.
        if (NewCoarseTime > pContext->FrameLength - SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF)
        {
            NewCoarseTime = pContext->FrameLength - SENSOR_BAYER_DEFAULT_MAX_COARSE_DIFF;
        }

        if (pContext->CoarseTime != NewCoarseTime)
        {
            ae.coarse_time = NewCoarseTime;
            ae.coarse_time_enable = NV_TRUE;

            // Calculate the new exposure based on the sensor and sensor settings.
            pContext->Exposure = ((NewCoarseTime -
                                   (NvF32)DIFF_INTEGRATION_TIME_OF_MODE) *
                                  (NvF32)LineLength) / Freq;
            pContext->CoarseTime = NewCoarseTime;
        }
    }

    if (ae.gain_enable==NV_TRUE || ae.coarse_time_enable==NV_TRUE ||
            ae.frame_length_enable==NV_TRUE) {
        ret = ioctl(pContext->camera_fd, OV5650_IOCTL_SET_GROUP_HOLD, &ae);
        if (ret < 0)
        {
            NvOsDebugPrintf("ioctl to set group hold failed %s\n", strerror(errno));
            return NV_FALSE;
        }
    }
    return NV_TRUE;
}
#endif

/**
 * SensorBayer_Open. Phase 1.
 * Initialize sensor bayer and its private context
 */
static NvBool SensorBayer_Open(NvOdmImagerHandle hImager)
{
    SensorBayerContext *pSensorBayerContext = NULL;
#if (BUILD_FOR_AOS == 1)
    NvOdmImagerI2cConnection *pI2c = NULL;
#endif

    if (!hImager || !hImager->pSensor)
        return NV_FALSE;

    pSensorBayerContext =
        (SensorBayerContext*)NvOdmOsAlloc(sizeof(SensorBayerContext));
    if (!pSensorBayerContext)
        goto fail;

    pSensorBayerContext->FrameErrorCount = 0;
    NvOsDebugPrintf("pSensorBayerContext->FrameErrorCount = 0x%x\n", pSensorBayerContext->FrameErrorCount);

    pSensorBayerContext->TestPatternMode = NV_FALSE;
    pSensorBayerContext->TestPattern = NvOdmImagerTestPattern_None;

    NvOdmOsMemset(pSensorBayerContext, 0, sizeof(SensorBayerContext));

#if (BUILD_FOR_AOS == 1)
    // Setup the i2c bit widths so we can call the
    // generalized write/read utility functions.
    // This is done here, since it will vary from sensor to sensor.
    pI2c = &pSensorBayerContext->I2c;
    pI2c->AddressBitWidth = 16;
    pI2c->DataBitWidth = 8;
#endif

    g_pSensorBayerSetModeSequenceList = g_SensorBayerSetModeSequenceList;
    pSensorBayerContext->NumModes =
        NV_ARRAY_SIZE(g_SensorBayerSetModeSequenceList);

    pSensorBayerContext->ModeIndex =
        pSensorBayerContext->NumModes; // invalid mode

    pSensorBayerContext->showSkipWriteGains = NV_TRUE;
    pSensorBayerContext->showSkipWriteExposure = NV_TRUE;

    /**
     * Phase 2. Initialize exposure and gains.
     */
    pSensorBayerContext->Exposure = -1.0; // invalid exposure

     // no need to convert gain's min/max to float for OV5650, because they are defined as float already.
    pSensorBayerContext->MaxGain = SENSOR_BAYER_DEFAULT_MAX_GAIN; // SENSOR_GAIN_TO_F32(SENSOR_BAYER_DEFAULT_MAX_GAIN);
    pSensorBayerContext->MinGain = SENSOR_BAYER_DEFAULT_MIN_GAIN; // SENSOR_GAIN_TO_F32(SENSOR_BAYER_DEFAULT_MIN_GAIN);
    /**
     * Phase 2 ends here.
     */

    pSensorBayerContext->PowerLevel = NvOdmImagerPowerLevel_Off;

    /* RightOnly requires NvOdmImagerSensorInterface_SerialB */
    pSensorBayerContext->StereoCameraMode = StereoCameraMode_Left;
    g_SensorBayerCaps.SensorOdmInterface = NvOdmImagerSensorInterface_SerialA;

    hImager->pSensor->pPrivateContext = pSensorBayerContext;
    return NV_TRUE;

fail:
    NvOdmOsFree(pSensorBayerContext);
    return NV_FALSE;
}


/**
 * SensorBayer_Close. Phase 1.
 * Free the sensor's private context
 */
static void SensorBayer_Close(NvOdmImagerHandle hImager)
{
    SensorBayerContext *pContext = NULL;

    if (!hImager || !hImager->pSensor || !hImager->pSensor->pPrivateContext)
        return;

    pContext = (SensorBayerContext*)hImager->pSensor->pPrivateContext;

    // cleanup
    NvOdmOsFree(pContext);
    hImager->pSensor->pPrivateContext = NULL;
}

/**
 * SensorBayer_GetCapabilities. Phase 1.
 * Returnt sensor bayer's capabilities
 */
static void
SensorBayer_GetCapabilities(
    NvOdmImagerHandle hImager,
    NvOdmImagerCapabilities *pCapabilities)
{
    // Copy the sensor caps from g_SensorBayerCaps
    NvOdmOsMemcpy(pCapabilities, &g_SensorBayerCaps,
        sizeof(NvOdmImagerCapabilities));
}

/**
 * SensorBayer_ListModes. Phase 1.
 * Return a list of modes that sensor bayer supports.
 * If called with a NULL ptr to pModes, then it just returns the count
 */
static void
SensorBayer_ListModes(
    NvOdmImagerHandle hImager,
    NvOdmImagerSensorMode *pModes,
    NvS32 *pNumberOfModes)
{
    SensorBayerContext *pContext =
        (SensorBayerContext*)hImager->pSensor->pPrivateContext;
    NvS32 i;

    if (pNumberOfModes)
    {
        *pNumberOfModes = (NvS32)pContext->NumModes;
        if (pModes)
        {
            // Copy the modes from g_pSensorBayerSetModeSequenceList
            for (i = 0; i < *pNumberOfModes; i++)
            {
                pModes[i] = g_pSensorBayerSetModeSequenceList[i].Mode;

                if (pContext->StereoCameraMode == StereoCameraMode_Stereo)
                        pModes[i].PixelAspectRatio /= 2.0;
            }
        }
    }
}

/**
 * SensorBayer_SetMode. Phase 1.
 * Set sensor bayer to the mode of the desired resolution and frame rate.
 */
static NvBool
SensorBayer_SetMode(
    NvOdmImagerHandle hImager,
    const SetModeParameters *pParameters,
    NvOdmImagerSensorMode *pSelectedMode,
    SetModeParameters *pResult)
{
    NvBool Status = NV_FALSE;
    SensorBayerContext *pContext =
        (SensorBayerContext*)hImager->pSensor->pPrivateContext;
    NvU32 Index;
    SensorBayerOV5650ModeDependentSettings *pModeSettings;

#if (BUILD_FOR_AOS == 0)
    NvU32 CoarseTime = 0;
    NvU32 FrameLength = 0;
    NvU16 Gain = 0;

    NV_DEBUG_PRINTF(("Setting resolution to %dx%d, exposure %f, gains %f\n",
        pParameters->Resolution.width, pParameters->Resolution.height,
        pParameters->Exposure, pParameters->Gains[0]));

    pContext->FrameErrorCount = 0;

    // Find the right entrys in g_pSensorBayerSetModeSequenceList that matches
    // the desired resolution and framerate
    for (Index = 0; Index < pContext->NumModes; Index++)
    {
        if ((pParameters->Resolution.width ==
             g_pSensorBayerSetModeSequenceList[Index].Mode.
             ActiveDimensions.width) &&
            (pParameters->Resolution.height ==
             g_pSensorBayerSetModeSequenceList[Index].Mode.
             ActiveDimensions.height))
             break;
    }

    // No match found
    if (Index == pContext->NumModes)
        return NV_FALSE;

    if (pSelectedMode)
    {
        *pSelectedMode = g_pSensorBayerSetModeSequenceList[Index].Mode;
    }

    // These hardcoded numbers are from the set mode sequence and this formula
    // is based on this sensor. Sensor Dependent.
    //
    pContext->PllMult = SENSOR_BAYER_DEFAULT_PLL_MULT;
    pContext->PllPreDiv = SENSOR_BAYER_DEFAULT_PLL_PRE_DIV;
    pContext->PllPosDiv = SENSOR_BAYER_DEFAULT_PLL_POS_DIV;
    pContext->PllVtPixDiv = SENSOR_BAYER_DEFAULT_PLL_VT_PIX_DIV;
    pContext->PllVtSysDiv = SENSOR_BAYER_DEFAULT_PLL_VT_SYS_DIV;

    pContext->VtPixClkFreqHz =
        (NvU32)((pContext->SensorInputClockkHz * 1000 * pContext->PllMult) /
                (pContext->PllPreDiv * pContext->PllVtPixDiv *
                 pContext->PllVtSysDiv));

    pModeSettings =
        (SensorBayerOV5650ModeDependentSettings*)
        g_pSensorBayerSetModeSequenceList[Index].pModeDependentSettings;

    // set initial line lenth, frame length, coarse time, and max/min of frame length
    pContext->LineLength             = pModeSettings->LineLength;
    pContext->FrameLength            = pModeSettings->FrameLength;
    pContext->CoarseTime             = pModeSettings->CoarseTime;
    pContext->MaxFrameLength         = SENSOR_BAYER_DEFAULT_MAX_FRAME_LENGTH;
    pContext->MinFrameLength         = pModeSettings->MinFrameLength;
    pContext->InherentGain           = pModeSettings->InherentGain;
    pContext->SupportsBinningControl = pModeSettings->SupportsBinningControl;
    pContext->BinningControlEnabled  = pModeSettings->BinningControlEnabled;
    pContext->BinningControlDirty    = NV_FALSE;

    pContext->Exposure    =
              (((NvF32)pContext->CoarseTime - DIFF_INTEGRATION_TIME_OF_MODE) *
               (NvF32)pContext->LineLength) /(NvF32)pContext->VtPixClkFreqHz;
    pContext->MaxExposure =
              (((NvF32)SENSOR_BAYER_DEFAULT_MAX_EXPOSURE_COARSE - DIFF_INTEGRATION_TIME_OF_MODE) *
               (NvF32)pContext->LineLength ) / (NvF32)pContext->VtPixClkFreqHz;
    pContext->MinExposure =
              (((NvF32)SENSOR_BAYER_DEFAULT_MIN_EXPOSURE_COARSE - DIFF_INTEGRATION_TIME_OF_MODE) *
               (NvF32)pContext->LineLength ) / (NvF32)pContext->VtPixClkFreqHz;

    pContext->FrameRate =
              (NvF32)pContext->VtPixClkFreqHz /
              (NvF32)(pContext->FrameLength * pContext->LineLength);
    pContext->MaxFrameRate =
              (NvF32)pContext->VtPixClkFreqHz /
              (NvF32)(pContext->MinFrameLength * pContext->LineLength);
    pContext->MinFrameRate =
              (NvF32)pContext->VtPixClkFreqHz /
              (NvF32)(SENSOR_BAYER_DEFAULT_MAX_FRAME_LENGTH * pContext->LineLength);


    pContext->Gains[0] = 1.0;
    pContext->Gains[1] = 1.0;
    pContext->Gains[2] = 1.0;
    pContext->Gains[3] = 1.0;

    if ((pParameters->Exposure != 0.0) && (pContext->TestPatternMode != NV_TRUE))
    {
        Status = SensorBayer_WriteExposure(pContext, &pParameters->Exposure,
                    &FrameLength, &CoarseTime);
        if (!Status)
        {
            NvOsDebugPrintf("SensorOV5650_WriteExposure failed\n");
        }
    }
    else
    {
        FrameLength = pModeSettings->FrameLength;
        CoarseTime = pModeSettings->CoarseTime;
    }

    if (pParameters->Gains[0] != 0.0 && pParameters->Gains[1] != 0.0 &&
        pParameters->Gains[2] != 0.0 && pParameters->Gains[3] != 0.0)
    {
        Status = SensorBayer_WriteGains(pContext, pParameters->Gains, &Gain);
        if (!Status)
        {
            NvOsDebugPrintf("SensorOV5650_WriteGains failed\n");
        }
    }

    int ret;
    struct ov5650_mode mode = {
        g_pSensorBayerSetModeSequenceList[Index].Mode.
            ActiveDimensions.width,
        g_pSensorBayerSetModeSequenceList[Index].Mode.
            ActiveDimensions.height,
            FrameLength, CoarseTime, Gain
    };
    ret = ioctl(pContext->camera_fd, OV5650_IOCTL_SET_MODE, &mode);
    if (ret < 0) {
        NvOsDebugPrintf("%s: ioctl to set mode failed %s\n", __func__,
            strerror(errno));
        return NV_FALSE;
    } else {
        Status = NV_TRUE;
    }

    /**
     * the following is Phase 2.
     */

    // Update sensor context after set mode
    NV_ASSERT(pContext->SensorInputClockkHz > 0); // UN-commented


    NV_DEBUG_PRINTF(("-------------SetMode---------------\n"));
    NV_DEBUG_PRINTF(("Exposure : %f (%f, %f)\n",
             pContext->Exposure,
             pContext->MinExposure,
             pContext->MaxExposure));
    NV_DEBUG_PRINTF(("Gain : %f (%f, %f)\n",
             pContext->Gains[1],
             pContext->MinGain,
             pContext->MaxGain));
    NV_DEBUG_PRINTF(("FrameRate : %f (%f, %f)\n",
             pContext->FrameRate,
             pContext->MinFrameRate,
             pContext->MaxFrameRate));

    if (pResult)
    {
        pResult->Resolution = g_pSensorBayerSetModeSequenceList[Index].
                              Mode.ActiveDimensions;
        pResult->Exposure = pContext->Exposure;
        NvOdmOsMemcpy(pResult->Gains, &pContext->Gains, sizeof(NvF32) * 4);
    }

    /**
     * Phase 2 ends here.
     */
    pContext->ModeIndex = Index;

    // Wait 1.5 frames for gains/exposure to take effect only after the first
    // time set mode. 1.5 is chosen so that camera driver have 0.5 frame time
    // to issue a capture request for the first good frame.
    if (pContext->SensorInitialized)
        NvOsSleepMS((NvU32)(1500.0 / (NvF32)(pContext->FrameRate)));

    pContext->SensorInitialized = NV_TRUE;

    if (pContext->TestPatternMode)
    {
        NvF32 Gains[4];
        NvU32 i;

        // reset gains
        for (i = 0; i < 4; i++)
            Gains[i] = pContext->MinGain;

        Status = SensorBayer_WriteGains(pContext, Gains, NULL);
        if (!Status)
            return NV_FALSE;

        // TODO: Make this more predictable (ideally, move into driver).
        NvOdmOsWaitUS(350 * 1000);
    }
#else
    DevCtrlReg16 OverrideList[MAX_OVERRIDE_LIST_SIZE];
    DevCtrlReg16 *pOverrideList = OverrideList;
    NvBool UseFastMode;

#if DEBUG_PRINTS
    NvOsDebugPrintf("Setting resolution to %dx%d, exposure %f, gains %f\n",
        pParameters->Resolution.width, pParameters->Resolution.height,
        pParameters->Exposure, pParameters->Gains[0]);
#endif

    // Find the right entrys in g_pSensorBayerSetModeSequenceList that matches
    // the desired resolution and framerate
    for (Index = 0; Index < pContext->NumModes; Index++)
    {
        if ((pParameters->Resolution.width ==
             g_pSensorBayerSetModeSequenceList[Index].Mode.
             ActiveDimensions.width) &&
            (pParameters->Resolution.height ==
             g_pSensorBayerSetModeSequenceList[Index].Mode.
             ActiveDimensions.height))
             break;
    }

    // No match found
    if (Index == pContext->NumModes)
        return NV_FALSE;

    if (pSelectedMode)
    {
        *pSelectedMode = g_pSensorBayerSetModeSequenceList[Index].Mode;
    }

    if (pContext->ModeIndex == Index)
        return NV_TRUE;

    // These hardcoded numbers are from the set mode sequence and this formula
    // is based on this sensor. Sensor Dependent.
    //

    if (hImager->pSensor->GUID == SENSOR_BYRST_OV5650_GUID) // Stereo board
    {
        pContext->PllMult = 12;
        pContext->PllPreDiv = 1.5;
        pContext->PllPosDiv = 1;
        pContext->PllVtPixDiv =  1;
    }
    else
    {
        pContext->PllMult = SENSOR_BAYER_DEFAULT_PLL_MULT;
        pContext->PllPreDiv = SENSOR_BAYER_DEFAULT_PLL_PRE_DIV;
        pContext->PllPosDiv = SENSOR_BAYER_DEFAULT_PLL_POS_DIV;
        pContext->PllVtPixDiv = SENSOR_BAYER_DEFAULT_PLL_VT_PIX_DIV;
    }

    pContext->PllVtSysDiv = SENSOR_BAYER_DEFAULT_PLL_VT_SYS_DIV;

    pContext->VtPixClkFreqHz =
        (NvU32)((pContext->SensorInputClockkHz * 1000 * pContext->PllMult) /
                (pContext->PllPreDiv * pContext->PllVtPixDiv *
                 pContext->PllVtSysDiv));

    pModeSettings =
        (SensorBayerOV5650ModeDependentSettings*)
        g_pSensorBayerSetModeSequenceList[Index].pModeDependentSettings;

    // set initial line lenth, frame length, coarse time, and max/min of frame length
    pContext->LineLength             = pModeSettings->LineLength;
    pContext->FrameLength            = pModeSettings->FrameLength;
    pContext->CoarseTime             = pModeSettings->CoarseTime;
    pContext->MaxFrameLength         = SENSOR_BAYER_DEFAULT_MAX_FRAME_LENGTH;
    pContext->MinFrameLength         = pModeSettings->MinFrameLength;
    pContext->InherentGain           = pModeSettings->InherentGain;
    pContext->SupportsBinningControl = pModeSettings->SupportsBinningControl;
    pContext->BinningControlEnabled  = pModeSettings->BinningControlEnabled;
    pContext->BinningControlDirty    = NV_FALSE;

    pContext->Exposure    =
              (((NvF32)pContext->CoarseTime) *
               (NvF32)pContext->LineLength) /(NvF32)pContext->VtPixClkFreqHz;
    pContext->MaxExposure =
              (((NvF32)SENSOR_BAYER_DEFAULT_MAX_EXPOSURE_COARSE) *
               (NvF32)pContext->LineLength ) / (NvF32)pContext->VtPixClkFreqHz;
    pContext->MinExposure =
              (((NvF32)SENSOR_BAYER_DEFAULT_MIN_EXPOSURE_COARSE) *
               (NvF32)pContext->LineLength ) / (NvF32)pContext->VtPixClkFreqHz;

    pContext->FrameRate =
              (NvF32)pContext->VtPixClkFreqHz /
              (NvF32)(pContext->FrameLength * pContext->LineLength);
    pContext->MaxFrameRate =
              (NvF32)pContext->VtPixClkFreqHz /
              (NvF32)(pContext->MinFrameLength * pContext->LineLength);
    pContext->MinFrameRate =
              (NvF32)pContext->VtPixClkFreqHz /
              (NvF32)(SENSOR_BAYER_DEFAULT_MAX_FRAME_LENGTH * pContext->LineLength);


    pContext->Gains[0] = 1.0;
    pContext->Gains[1] = 1.0;
    pContext->Gains[2] = 1.0;
    pContext->Gains[3] = 1.0;

    if (pParameters->Exposure != 0.0)
    {
        Status = SensorBayer_WriteExposure(pContext, &pParameters->Exposure, &pOverrideList);
        if (!Status)
        {
            NV_DEBUG_PRINTF(("SensorOV5650_WriteExposure failed\n"));
        }
    }

    if (pParameters->Gains[0] != 0.0 && pParameters->Gains[1] != 0.0 &&
        pParameters->Gains[2] != 0.0 && pParameters->Gains[3] != 0.0)
    {
        Status = SensorBayer_WriteGains(pContext, pParameters->Gains,
            &pOverrideList);
        if (!Status)
        {
            NV_DEBUG_PRINTF(("SensorOV5650_WriteGains failed\n"));
        }
    }

    UseFastMode = pModeSettings->SupportsFastMode &&
                 pContext->FastSetMode && pContext->SensorInitialized;
    // i2c writes for the set mode sequence
    Status = WriteI2CSequenceOverride(&pContext->I2c,
                 g_pSensorBayerSetModeSequenceList[Index].pSequence,
                 UseFastMode, OverrideList, pOverrideList - OverrideList);
    if (!Status)
        return NV_FALSE;

    if (pContext->I2cR.hOdmI2c)
    {
        Status = WriteI2CSequenceOverride(&pContext->I2cR,
                     g_pSensorBayerSetModeSequenceList[Index].pSequence,
                     pContext->FastSetMode && pContext->SensorInitialized,
                     OverrideList, pOverrideList - OverrideList);
        if (!Status)
            return NV_FALSE;
    }

    /**
     * the following is Phase 2.
     */
    pContext->SensorInitialized = NV_TRUE;

    // Update sensor context after set mode
    NV_ASSERT(pContext->SensorInputClockkHz > 0); // UN-commented

#if DEBUG_PRINTF
    NvOsDebugPrintf("-------------SetMode---------------\r\n");
    NvOsDebugPrintf("Exposure : %f (%f, %f)\r\n",
                 pContext->Exposure,
                 pContext->MinExposure,
                 pContext->MaxExposure);
    NvOsDebugPrintf("Gain : %f (%f, %f)\r\n",
                 pContext->Gains[1],
                 pContext->MinGain,
                 pContext->MaxGain);
    NvOsDebugPrintf("FrameRate : %f (%f, %f)\r\n",
                pContext->FrameRate,
                pContext->MinFrameRate,
                pContext->MaxFrameRate);
#endif

    if (pResult)
    {
        pResult->Resolution = g_pSensorBayerSetModeSequenceList[Index].
                              Mode.ActiveDimensions;
        pResult->Exposure = pContext->Exposure;
        NvOdmOsMemcpy(pResult->Gains, &pContext->Gains, sizeof(NvF32) * 4);
    }

    /**
     * Phase 2 ends here.
     */
    pContext->ModeIndex = Index;

    // Wait 2 frames for gains/exposure to take effect.
    NvOsSleepMS((NvU32)(2000.0 / (NvF32)(pContext->FrameRate)));

    if (pContext->TestPatternMode)
    {
        NvF32 Gains[4];
        NvU32 i;

        // reset gains
        for (i = 0; i < 4; i++)
            Gains[i] = pContext->MinGain;

        Status = SensorBayer_WriteGains(pContext, Gains, NULL);
        if (!Status)
            return NV_FALSE;

        // Enable the test mode register
        // NVODM_WRITE_RETURN_ON_ERROR(&pContext->I2c, 0x3508, 0x80);
        // Enable color bar and select the standard color bar
        // NVODM_WRITE_RETURN_ON_ERROR(&pContext->I2c, 0x3301, 0xF2);
        NvOdmOsWaitUS(350 * 1000);
    }
    if (hImager->pSensor->GUID == SENSOR_BYRST_OV5650_GUID) // Sync two sensors
    {
        NvOdmGpioSetState(pContext->GpioConfig.hGpio,
                          pContext->GpioConfig.hPin[pContext->GpioConfig.Gpios[NvOdmImagerGpio_Powerdown].HandleIndex],
                          NvOdmGpioPinActiveState_High);
        NvOdmOsWaitUS(1000);
        NvOdmGpioSetState(pContext->GpioConfig.hGpio,
                          pContext->GpioConfig.hPin[pContext->GpioConfig.Gpios[NvOdmImagerGpio_Powerdown].HandleIndex],
                          NvOdmGpioPinActiveState_Low);
        NvOdmOsWaitUS(1000);
    }
#endif
    return Status;
}


/**
 * SensorBayer_SetPowerLevel. Phase 1
 * Set the sensor's power level
 */
static NvBool
SensorBayer_SetPowerLevel(
    NvOdmImagerHandle hImager,
    NvOdmImagerPowerLevel PowerLevel)
{
    NvBool Status = NV_TRUE;
    NvBool FocuserStatus = NV_FALSE;
    SensorBayerContext *pContext =
        (SensorBayerContext*)hImager->pSensor->pPrivateContext;
    int ret;

#if (BUILD_FOR_AOS == 0)
    NV_DEBUG_PRINTF(("SensorBayer_SetPowerLevel %d\n", PowerLevel));

    if (pContext->PowerLevel == PowerLevel)
        return NV_TRUE;

    switch(PowerLevel)
    {
        case NvOdmImagerPowerLevel_On:
            pContext->camera_fd = open("/dev/ov5650", O_RDWR);
            if (pContext->camera_fd < 0) {
                NvOsDebugPrintf("Can not open camera device: %s\n",
                    strerror(errno));
                return NV_FALSE;
            } else {
                NV_DEBUG_PRINTF(("Camera fd open as: %d\n", pContext->camera_fd));
                Status = NV_TRUE;
            }

            ret = ioctl(pContext->camera_fd, OV5650_IOCTL_SET_CAMERA_MODE,
                    pContext->StereoCameraMode);
            if (ret) {
                Status = NV_FALSE;
                NvOdmOsDebugPrintf("[IMAGER] ioctl to"
                    "OV5650_IOCTL_SET_CAMERA_MODE failed %s\n",
                        strerror(errno));
            }
            break;

        case NvOdmImagerPowerLevel_Standby:
            Status = NV_TRUE;
            break;

        case NvOdmImagerPowerLevel_SyncSensors:
            ret = ioctl(pContext->camera_fd, OV5650_IOCTL_SYNC_SENSORS, 0);
            if (ret)
                Status = NV_FALSE;
            break;

        case NvOdmImagerPowerLevel_Off:
            close(pContext->camera_fd);
            pContext->camera_fd = -1;
            // PowerOff the associated focuser as well.
            if (hImager->pFocuser)
                FocuserStatus = hImager->pFocuser->pfnSetPowerLevel(
                                    hImager, NvOdmImagerPowerLevel_Off);
            if (!FocuserStatus)
                  NvOsDebugPrintf("Cannot power off focuser\n");
            break;

        default:
            NV_ASSERT("!Unknown power level\n");
            Status = NV_FALSE;
            break;
    }
#else
    const NvOdmPeripheralConnectivity *pConnections;

    if (pContext->PowerLevel == PowerLevel)
        return NV_TRUE;

    NV_ASSERT(hImager->pSensor->pConnections);
    pConnections = hImager->pSensor->pConnections;

    switch(PowerLevel)
    {
        case NvOdmImagerPowerLevel_On:
            Status = SetPowerLevelWithPeripheralConnectivityHelper(pConnections,
                         &pContext->I2c, &pContext->I2cR, &pContext->GpioConfig,
                         NvOdmImagerPowerLevel_On);
            if (!Status)
                return NV_FALSE;
            break;

        case NvOdmImagerPowerLevel_Standby:
            Status = SetPowerLevelWithPeripheralConnectivityHelper(pConnections,
                         &pContext->I2c, &pContext->I2cR, &pContext->GpioConfig,
                         NvOdmImagerPowerLevel_Standby);
            break;

        case NvOdmImagerPowerLevel_Off:
            Status = SetPowerLevelWithPeripheralConnectivityHelper(pConnections,
                         &pContext->I2c, &pContext->I2cR, &pContext->GpioConfig,
                         NvOdmImagerPowerLevel_Off);
            break;

        default:
            NV_ASSERT("!Unknown power level\n");
            Status = NV_FALSE;
            break;
    }
#endif

    if (Status)
        pContext->PowerLevel = PowerLevel;

    return Status;
}

/**
 * SensorBayer_SetParameter. Phase 2.
 * Set sensor specific parameters.
 * For Phase 1. This can return NV_TRUE.
 */
static NvBool
SensorBayer_SetParameter(
    NvOdmImagerHandle hImager,
    NvOdmImagerParameter Param,
    NvS32 SizeOfValue,
    const void *pValue)
{
    NvBool Status = NV_TRUE;
    SensorBayerContext *pContext =
        (SensorBayerContext*)hImager->pSensor->pPrivateContext;

    NV_DEBUG_PRINTF(("SetParameter(): %x\n", Param));

    switch(Param)
    {
        // Phase 2.
        case NvOdmImagerParameter_StereoCameraMode:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue,
                sizeof(NvOdmImagerStereoCameraMode));

            if (hImager->pSensor->GUID == SENSOR_BYRST_OV5650_GUID)
            {
                NvOdmImagerStereoCameraMode CameraMode = *(NvOdmImagerStereoCameraMode *)pValue;
                pContext->StereoCameraMode = CameraMode;

                // set the Number Of Camera sensor Modes
                g_pSensorBayerSetModeSequenceList = g_SensorBayerSetModeSequenceStereoList;
                pContext->NumModes =
                    NV_ARRAY_SIZE(g_SensorBayerSetModeSequenceStereoList);

                pContext->ModeIndex =
                    pContext->NumModes; // invalid mode

            }
            else
            {
                Status = NV_FALSE;
            }
            break;

        case NvOdmImagerParameter_SensorGroupHold:
        {
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvOdmImagerSensorAE));
#if (BUILD_FOR_AOS == 0)
            Status = SensorBayer_GroupHold(pContext, (NvOdmImagerSensorAE *)pValue);
#endif
        }
        break;

        case NvOdmImagerParameter_SensorExposure:
        {
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvF32));

#if (BUILD_FOR_AOS == 0)
            Status = SensorBayer_WriteExposure(pContext, (NvF32*)pValue, NULL, NULL);
#else
            Status = SensorBayer_WriteExposure(pContext, (NvF32*)pValue, NULL);
#endif
        }
        break;

        // Phase 2.
        case NvOdmImagerParameter_SensorGain:
        {
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, 4 * sizeof(NvF32));
            Status = SensorBayer_WriteGains(pContext, pValue, NULL);
        }
        break;

        // Phase 2.
        case NvOdmImagerParameter_SensorInputClock:
        {
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue,
                                             sizeof(NvOdmImagerClockProfile));
            Status = SensorBayer_SetInputClock(
                      pContext,
                      ((NvOdmImagerClockProfile*)pValue));
        }
        break;

        // Phase 3.
        case NvOdmImagerParameter_OptimizeResolutionChange:
        {
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvBool));

#if (BUILD_FOR_AOS == 1)
            pContext->FastSetMode = *((NvBool*)pValue);
#endif
            Status = NV_TRUE;
        }
        break;

        // This is optional but nice to have.
        case NvOdmImagerParameter_SelfTest:
        {
            // Not Implemented.
        }
        break;

        // Phase 3.
        case NvOdmImagerParameter_MaxSensorFrameRate:
        {
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvF32));
            pContext->RequestedMaxFrameRate = *((NvF32*)pValue);
        }
        break;
        case NvOdmImagerParameter_TestMode:
        {
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvBool));
            pContext->TestPatternMode = *((NvBool *)pValue);

            // If Sensor is initialized then only program the test mode registers
            // else just save the test mode in pContext->TestPatternMode and once
            // the sensor gets initialized in SensorOV5650_SetMode() there itself
            // program the test mode registers.
            if(pContext->SensorInitialized)
            {
                if (pContext->TestPatternMode)
                {
                    NvF32 Gains[4];
                    NvU32 i;

                    // reset gains
                    for (i = 0; i < 4; i++)
                    Gains[i] = pContext->MinGain;

                    Status = SensorBayer_WriteGains(pContext, Gains, NULL);
                    if (!Status)
                        return NV_FALSE;

                }

                // TODO: Make this more predictable (ideally, move into driver).
                NvOdmOsWaitUS(350 * 1000);
            }
        }
        break;

#if 0
        case NvOdmImagerParameter_Reset:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue,
                sizeof(NvOdmImagerReset));
            switch(*(NvOdmImagerReset*)pValue)
            {
                case NvOdmImagerReset_Hard:
                    Status = SensorBayer_HardReset(hImager);
            break;

                case NvOdmImagerReset_Soft:
                default:
                    NV_ASSERT(!"Not Implemented!");
                    Status = NV_FALSE;
            }
            break;
#endif

        case NvOdmImagerParameter_SensorInherentGainAtResolution:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvBool));
            CHECK_SENSOR_RETURN_NOT_INITIALIZED(pContext);

            {
                if ( !((SensorBayerOV5650ModeDependentSettings *)g_pSensorBayerSetModeSequenceList[pContext->ModeIndex].pModeDependentSettings)->SupportsBinningControl )
                {
                    // does not allow to set binning mode if current mode is a full resolution.
                    NV_DEBUG_PRINTF(("can't set binning mode because current mode is a full resolution."));
                    Status = NV_FALSE;
                }
                else
                {
                    NvBool flag = *((NvBool *)pValue);
                    if (pContext->BinningControlEnabled != flag)
                    {
                        pContext->BinningControlEnabled = flag;
                        pContext->BinningControlDirty = NV_TRUE;
                    }
                }
            }
            break;

        default:
#if (BUILD_FOR_AOS == 0)
            NV_DEBUG_PRINTF(("SetParameter(): %d not supported\n", Param));
#else
            //NvOsDebugPrintf("SetParameter(): %x not supported\r\n", Param);
            Status = NV_FALSE;
#endif
            break;
        }
        return Status;
}

/**
 * SensorBayer_GetParameter. Phase 1.
 * Get sensor specific parameters
 */
static NvBool
SensorBayer_GetParameter(
    NvOdmImagerHandle hImager,
    NvOdmImagerParameter Param,
    NvS32 SizeOfValue,
    void *pValue)
{
    NvBool Status = NV_TRUE;

    NvF32 *pFloatValue = pValue;
    SensorBayerContext *pContext =
        (SensorBayerContext*)hImager->pSensor->pPrivateContext;
    NV_DEBUG_PRINTF(("GetParameter(): %d\n", Param));


    switch(Param)
    {
        // Phase 1.
        case NvOdmImagerParameter_StereoCapable:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue,
                sizeof(NvBool));
            {
                NvBool *pBL = (NvBool*)pValue;
                *pBL = ((hImager->pSensor->GUID == SENSOR_BYRST_OV5650_GUID) ? NV_TRUE : NV_FALSE);
            }
            break;

        case NvOdmImagerParameter_StereoCameraMode:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue,
                sizeof(NvOdmImagerStereoCameraMode));
            if (hImager->pSensor->GUID == SENSOR_BYRST_OV5650_GUID)
            {
                NvOdmImagerStereoCameraMode *pMode = (NvOdmImagerStereoCameraMode*)pValue;
                *pMode = pContext->StereoCameraMode;
            }
            else
            {
                Status = NV_FALSE;
            }
            break;

        case NvOdmImagerParameter_SensorType:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvOdmImagerSensorType));
            *((NvOdmImagerSensorType*)pValue) = NvOdmImager_SensorType_Raw;
            break;

        case NvOdmImagerParameter_CalibrationData:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue,
                sizeof(NvOdmImagerCalibrationData));
            {
                NvOdmImagerCalibrationData *pCalibration =
                        (NvOdmImagerCalibrationData*)pValue;
                pCalibration->CalibrationData = pSensorCalibrationData;
                pCalibration->NeedsFreeing = NV_FALSE;
            }
            break;

        // Phase 1, it can return min = max = 1.0f
        // Phase 2, return the real numbers.
        case NvOdmImagerParameter_SensorExposureLimits:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, 2 * sizeof(NvF32));

            pFloatValue[0] = pContext->MinExposure;
            pFloatValue[1] = pContext->MaxExposure;
            break;

        // Phase 1, it can return min = max = 1.0f
        // Phase 2, return the real numbers.
        case NvOdmImagerParameter_SensorGainLimits:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, 2 * sizeof(NvF32));
            pFloatValue[0] = pContext->MinGain;
            pFloatValue[1] = pContext->MaxGain;
            break;

        case NvOdmImagerParameter_FocalLength:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvF32));
            ((NvF32*)pValue)[0] = LENS_FOCAL_LENGTH;
            break;

        // Get the sensor status. This is optional but nice to have.
        case NvOdmImagerParameter_DeviceStatus:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue,
                sizeof(NvOdmImagerDeviceStatus));
            {
                NvOdmImagerDeviceStatus *pStatus;
                pStatus = (NvOdmImagerDeviceStatus *)pValue;
#if (BUILD_FOR_AOS == 0)
                {
                    uint16_t status;
                    int ret;
                    SetModeParameters Parameters;

                    ret = ioctl(pContext->camera_fd, OV5650_IOCTL_GET_STATUS, &status);
                    if (ret < 0)
                        NvOsDebugPrintf("ioctl to gets status failed "
                            "%s\n", strerror(errno));
                    /* Assume this is status request due to a frame timeout */
                    pContext->FrameErrorCount++;
                    /* Multiple errors (in succession?) */
                    if ( pContext->FrameErrorCount > 4) {
                        pContext->FrameErrorCount = 0;
                        /* sensor has reset or locked up (ESD discharge?) */
                        /* (status = reg 0x3103 = PWUP_DIS, 0x91 is reset state */
                        /* Hard reset the sensor and reconfigure it */
                        NvOsDebugPrintf("Bad sensor state, reset and reconfigure"
                            "%s\n", strerror(status));
                        SensorBayer_SetPowerLevel(hImager, NvOdmImagerPowerLevel_Off);
                        SensorBayer_SetPowerLevel(hImager, NvOdmImagerPowerLevel_On);
                        Parameters.Resolution.width =
                            g_pSensorBayerSetModeSequenceList[pContext->ModeIndex].Mode.ActiveDimensions.width;
                        Parameters.Resolution.height =
                            g_pSensorBayerSetModeSequenceList[pContext->ModeIndex].Mode.ActiveDimensions.height;
                        Parameters.Exposure = pContext->Exposure;
                        Parameters.Gains[0] = pContext->Gains[0];
                        Parameters.Gains[1] = pContext->Gains[1];
                        Parameters.Gains[2] = pContext->Gains[2];
                        Parameters.Gains[3] = pContext->Gains[3];
                        SensorBayer_SetMode(hImager,&Parameters,NULL,&Parameters);
                    }
                    pStatus->Count = 1;
                }
#else
                {
                    NvOdmImagerI2cConnection *pI2c = &pContext->I2c;
                    // Pick your own useful registers to use for debug
                    // If the camera hangs, these register values are printed
                    // Sensor Dependent.
                    NvOdmImagerI2cRead(pI2c, 0x0002, &pStatus->Values[0]);
                    pStatus->Count = 1;
                }
#endif
            }
            break;

        // Phase 1, it can return min = max = 10.0f
        //          (the fps in g_pSensorBayerSetModeSequenceList)
        // Phase 2, return the real numbers
        case NvOdmImagerParameter_SensorFrameRateLimits:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue,
                2 * sizeof(NvF32));

            pFloatValue[0] = pContext->MinFrameRate;
            pFloatValue[1] = pContext->MaxFrameRate;

            break;

        // Phase 1, it can return 1.0f
        // Phase 2, return the real numbers.
        case NvOdmImagerParameter_SensorFrameRate:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvF32));

            pFloatValue[0] = pContext->FrameRate;
            break;

        // Get the override config data.
        case NvOdmImagerParameter_CalibrationOverrides:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue,
                sizeof(NvOdmImagerCalibrationData));
            {
                NvOdmImagerCalibrationData *pCalibration =
                    (NvOdmImagerCalibrationData*)pValue;

                pCalibration->CalibrationData =
                    LoadOverridesFile(pOverrideFiles,
                        (sizeof(pOverrideFiles)/sizeof(pOverrideFiles[0])));
                pCalibration->NeedsFreeing = (pCalibration->CalibrationData != NULL);
                Status = pCalibration->NeedsFreeing;
            }
            break;

        // Get the factory calibration data.
        case NvOdmImagerParameter_FactoryCalibrationData:
            {
                Status = LoadBlobFile(pFactoryBlobFiles,
                                    sizeof(pFactoryBlobFiles)/sizeof(pFactoryBlobFiles[0]),
                                    pValue, SizeOfValue);
            }
            break;

        case NvOdmImagerParameter_SensorID:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvU8) * 16);

#if (BUILD_FOR_AOS == 0)
            // code to read the ID from sensor
            {
                struct ov5650_sensordata sensor_data;
                int ret, num;
                ret = ioctl(pContext->camera_fd, OV5650_IOCTL_GET_SENSORDATA, &sensor_data);
                if (ret < 0)
                {
                    NvOsDebugPrintf("ioctl to get sensor data failed %s\n", strerror(errno));
                    Status = NV_FALSE;
                }
                else
                {
                    NvOdmOsMemset(pValue, 0, SizeOfValue);
                    num = SizeOfValue > (int)sensor_data.fuse_id_size?
                                    (int)sensor_data.fuse_id_size : SizeOfValue;
                    NvOdmOsMemcpy(pValue, sensor_data.fuse_id, sizeof(NvU8) * num);
                    Status = NV_TRUE;
                }
            }
#endif
            break;

        case NvOdmImagerParameter_RegionUsedByCurrentResolution:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue,
                sizeof(NvOdmImagerRegion));


            if (pContext->ModeIndex >= pContext->NumModes)
            {
                // No info available until we've actually set a mode.
                return NV_FALSE;
            }
            else
            {
                NvOdmImagerRegion *pRegion = (NvOdmImagerRegion*)pValue;

                pRegion->RegionStart.x = 0;
                pRegion->RegionStart.y = 0;

                if (pContext->ModeIndex == 1)
                {
                    pRegion->xScale = 2;
                    pRegion->yScale = 2;
                }
                else
                {
                    pRegion->xScale = 1;
                    pRegion->yScale = 1;
                }
            }
            break;
        case NvOdmImagerParameter_LinesPerSecond:
            {
                NvF32 *pFloat = (NvF32 *)pValue;
                NvF32 fps; // frames per second
                NvF32 lines; // lines per frame
                fps = (NvF32)pContext->VtPixClkFreqHz /
                    (NvF32)(pContext->FrameLength * pContext->LineLength);
                lines = (NvF32)g_pSensorBayerSetModeSequenceList
                    [pContext->ModeIndex].Mode.ActiveDimensions.height;
                *pFloat = fps * lines;
            }
            break;

        case NvOdmImagerParameter_SensorActiveRegionReadOutTime:
            {
                NvF32 regionReadOutTime;
                NvF32 *pFloat = (NvF32 *)pValue;
                SensorBayerOV5650ModeDependentSettings *pModeSettings =
                (SensorBayerOV5650ModeDependentSettings*)
                    g_pSensorBayerSetModeSequenceList[pContext->ModeIndex].pModeDependentSettings;
                NvF32 height = (NvF32)(g_pSensorBayerSetModeSequenceList
                    [pContext->ModeIndex].Mode.ActiveDimensions.height);
                regionReadOutTime = (height * pModeSettings->LineLength) / (NvF32)pContext->VtPixClkFreqHz;
                *pFloat = regionReadOutTime;

            }
            break;

        case NvOdmImagerParameter_MaxSensorFrameRate:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvF32));
            pFloatValue[0] = pContext->RequestedMaxFrameRate;
            break;

        case NvOdmImagerParameter_ExpectedValues:
            break;

        case NvOdmImagerParameter_SensorGain:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, 4 * sizeof(NvF32));

            NvOdmOsMemcpy(pValue, pContext->Gains, sizeof(NvF32) * 4);
            break;

        case NvOdmImagerParameter_SensorGroupHold:
            {
                NvBool *grouphold = (NvBool *) pValue;
                CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvBool));
                *grouphold = NV_TRUE;
                break;
            }

        case NvOdmImagerParameter_SensorExposure:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvF32));

            NvOdmOsMemcpy(pValue, &pContext->Exposure, sizeof(NvF32));
            break;

        case NvOdmImagerParameter_SensorFrameRateLimitsAtResolution:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue,
                sizeof(NvOdmImagerFrameRateLimitAtResolution));

            CHECK_SENSOR_RETURN_NOT_INITIALIZED(pContext);

            {
                NvOdmImagerFrameRateLimitAtResolution *pData;
                NvU32 Index;
                NvBool MatchFound = NV_FALSE;
                SensorBayerOV5650ModeDependentSettings *pModeSettings = NULL;

                pData = (NvOdmImagerFrameRateLimitAtResolution *)pValue;
                pData->MinFrameRate = 0.0f;
                pData->MaxFrameRate = 0.0f;

                MatchFound =
                    SensorOV5650_ChooseModeIndex(pContext, pData->Resolution,
                    &Index);

                if (!MatchFound)
                {
                    Status = NV_FALSE;
                    break;
                }

                pModeSettings = (SensorBayerOV5650ModeDependentSettings*)
                    g_pSensorBayerSetModeSequenceList[Index].pModeDependentSettings;


                pData->MaxFrameRate = (NvF32)pContext->VtPixClkFreqHz /
                                      (NvF32)(pModeSettings->FrameLength *
                                              pModeSettings->LineLength);
                pData->MinFrameRate =
                    (NvF32)pContext->VtPixClkFreqHz /
                    (NvF32)(SENSOR_BAYER_DEFAULT_MAX_FRAME_LENGTH *
                            pModeSettings->LineLength);
            }
            break;

        case NvOdmImagerParameter_SensorInherentGainAtResolution:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue,
                sizeof(NvOdmImagerInherentGainAtResolution));

            CHECK_SENSOR_RETURN_NOT_INITIALIZED(pContext);

            {
                NvOdmImagerInherentGainAtResolution *pData;
                NvU32 Index;
                NvBool MatchFound = NV_FALSE;
                SensorBayerOV5650ModeDependentSettings *pModeSettings = NULL;

                pData = (NvOdmImagerInherentGainAtResolution *)pValue;
                pData->InherentGain           = pContext->InherentGain;
                pData->SupportsBinningControl = pContext->SupportsBinningControl;
                pData->BinningControlEnabled  = pContext->BinningControlEnabled;

                if (pData->Resolution.width == 0 && pData->Resolution.height == 0)
                    break; // They just wanted the current value

                MatchFound =
                    SensorOV5650_ChooseModeIndex(pContext, pData->Resolution,
                    &Index);

                if (!MatchFound)
                {
                    Status = NV_FALSE;
                    break;
                }

                pModeSettings = (SensorBayerOV5650ModeDependentSettings*)
                    g_pSensorBayerSetModeSequenceList[Index].pModeDependentSettings;

                pData->InherentGain           = pModeSettings->InherentGain;
                pData->SupportsBinningControl = pModeSettings->SupportsBinningControl;
                pData->BinningControlEnabled  = pContext->BinningControlEnabled;
            }
            break;

#if (BUILD_FOR_AOS == 0)
        case NvOdmImagerParameter_SensorExposureLatchTime:
            // FIXME: implement this
            break;
#else
        // This parameter tells the autoexposure algorithm that the changes to gain
        // or exposure registers in OV5650 take 2 frames to take effect.
        case NvOdmImagerParameter_SensorExposureLatchTime:
             CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue,
                 sizeof(NvU32));
             {
                NvU32 *pUIntValue = (NvU32*)pValue;
                *pUIntValue = 2;
             }
            break;
#endif
        case NvOdmImagerParameter_HorizontalViewAngle:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvF32));
            pFloatValue[0] = LENS_HORIZONTAL_VIEW_ANGLE;
            break;

        case NvOdmImagerParameter_VerticalViewAngle:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvF32));
            pFloatValue[0] = LENS_VERTICAL_VIEW_ANGLE;
            break;

        case NvOdmImagerParameter_BracketCaps:
            CHECK_PARAM_SIZE_RETURN_MISMATCH(SizeOfValue, sizeof(NvOdmImagerBracketConfiguration));
            {
                NvOdmImagerBracketConfiguration *pData;
                pData = (NvOdmImagerBracketConfiguration *)pValue;
                pData->FlushCount = 2;
                pData->InitialIntraFrameSkip = 0;
                pData->SteadyStateIntraFrameSkip = 2;
                pData->SteadyStateFrameNumer = 3;
            }
            break;

        // Phase 3.
        default:
            NV_DEBUG_PRINTF(("GetParameter(): %d not supported\n", Param));
            Status = NV_FALSE;
            break;
    }

    return Status;
}

/**
 * SensorBayer_GetPowerLevel. Phase 3.
 * Get the sensor's current power level
 */
static void
SensorBayer_GetPowerLevel(
    NvOdmImagerHandle hImager,
    NvOdmImagerPowerLevel *pPowerLevel)
{
    SensorBayerContext *pContext =
        (SensorBayerContext*)hImager->pSensor->pPrivateContext;

    *pPowerLevel = pContext->PowerLevel;
}

/**
 * SensorBayer_GetHal. Phase 1.
 * return the hal functions associated with sensor bayer
 */
NvBool SensorBayerOV5650_GetHal(NvOdmImagerHandle hImager)
{
    if (!hImager || !hImager->pSensor)
        return NV_FALSE;

    hImager->pSensor->pfnOpen = SensorBayer_Open;
    hImager->pSensor->pfnClose = SensorBayer_Close;
    hImager->pSensor->pfnGetCapabilities = SensorBayer_GetCapabilities;
    hImager->pSensor->pfnListModes = SensorBayer_ListModes;
    hImager->pSensor->pfnSetMode = SensorBayer_SetMode;
    hImager->pSensor->pfnSetPowerLevel = SensorBayer_SetPowerLevel;
    hImager->pSensor->pfnGetPowerLevel = SensorBayer_GetPowerLevel;
    hImager->pSensor->pfnSetParameter = SensorBayer_SetParameter;
    hImager->pSensor->pfnGetParameter = SensorBayer_GetParameter;

    return NV_TRUE;
}
