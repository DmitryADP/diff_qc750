/*
 * Copyright (c) 2007 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#include "nvodm_query_gpio.h"
#include "nvodm_services.h"
#include "nvrm_drf.h"
#include "nvodm_query_discovery.h"

#define NVODM_ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define NVODM_PORT(x) ((x) - 'a')

#define EEPROM_ID_E1206      0x0C06

static const NvOdmGpioPinInfo s_vi[] = {
    {NVODM_PORT('t'), 3, NvOdmGpioPinActiveState_High, NULL}, // EN_VDDIO_SD
};

static const NvOdmGpioPinInfo s_display[] = {

    /* Panel 0 -- sony vga */
    { NVODM_PORT('m'), 3, NvOdmGpioPinActiveState_Low, NULL },
    { NVODM_PORT('b'), 2, NvOdmGpioPinActiveState_Low, NULL },
    { NVODM_PORT('n'), 4, NvOdmGpioPinActiveState_Low, NULL },
    { NVODM_PORT('j'), 3, NvOdmGpioPinActiveState_Low, NULL },
    { NVODM_PORT('j'), 4, NvOdmGpioPinActiveState_Low, NULL },
    // this pin is not needed for ap15
    {NVODM_GPIO_INVALID_PORT, NVODM_GPIO_INVALID_PIN,
        NvOdmGpioPinActiveState_Low, NULL},

    /* Panel 1 -- samtek */
    {NVODM_GPIO_INVALID_PORT, NVODM_GPIO_INVALID_PIN,
        NvOdmGpioPinActiveState_Low, NULL},
    {NVODM_GPIO_INVALID_PORT, NVODM_GPIO_INVALID_PIN,
        NvOdmGpioPinActiveState_Low, NULL},
    {NVODM_GPIO_INVALID_PORT, NVODM_GPIO_INVALID_PIN,
        NvOdmGpioPinActiveState_Low, NULL},
    {NVODM_GPIO_INVALID_PORT, NVODM_GPIO_INVALID_PIN,
        NvOdmGpioPinActiveState_Low, NULL},

    /* Panel 2 -- sharp wvga */
    { NVODM_PORT('v'), 7, NvOdmGpioPinActiveState_Low, NULL },

    /* Panel 3 -- sharp qvga */
    { NVODM_PORT('n'), 6, NvOdmGpioPinActiveState_High, NULL },   // LCD_DC0
    { NVODM_PORT('n'), 4, NvOdmGpioPinActiveState_Low, NULL },    // LCD_CS0
    { NVODM_PORT('b'), 3, NvOdmGpioPinActiveState_Low, NULL },    // LCD_PCLK
    { NVODM_PORT('b'), 2, NvOdmGpioPinActiveState_Low, NULL },    // LCD_PWR0
    { NVODM_PORT('e'), 0, NvOdmGpioPinActiveState_High, NULL },   // LCD_D0
    { NVODM_PORT('e'), 1, NvOdmGpioPinActiveState_High, NULL },   // LCD_D1
    { NVODM_PORT('e'), 2, NvOdmGpioPinActiveState_High, NULL },   // LCD_D2
    { NVODM_PORT('e'), 3, NvOdmGpioPinActiveState_High, NULL },   // LCD_D3
    { NVODM_PORT('e'), 4, NvOdmGpioPinActiveState_High, NULL },   // LCD_D4
    { NVODM_PORT('e'), 5, NvOdmGpioPinActiveState_High, NULL },   // LCD_D5
    { NVODM_PORT('e'), 6, NvOdmGpioPinActiveState_High, NULL },   // LCD_D6
    { NVODM_PORT('e'), 7, NvOdmGpioPinActiveState_High, NULL },   // LCD_D7
    { NVODM_PORT('f'), 0, NvOdmGpioPinActiveState_High, NULL },   // LCD_D8
    { NVODM_PORT('f'), 1, NvOdmGpioPinActiveState_High, NULL },   // LCD_D9
    { NVODM_PORT('f'), 2, NvOdmGpioPinActiveState_High, NULL },   // LCD_D10
    { NVODM_PORT('f'), 3, NvOdmGpioPinActiveState_High, NULL },   // LCD_D11
    { NVODM_PORT('f'), 4, NvOdmGpioPinActiveState_High, NULL },   // LCD_D12
    { NVODM_PORT('f'), 5, NvOdmGpioPinActiveState_High, NULL },   // LCD_D13
    { NVODM_PORT('f'), 6, NvOdmGpioPinActiveState_High, NULL },   // LCD_D14
    { NVODM_PORT('f'), 7, NvOdmGpioPinActiveState_High, NULL },   // LCD_D15
    { NVODM_PORT('m'), 3, NvOdmGpioPinActiveState_High, NULL },   // LCD_D19

    /* Panel 4 -- auo */
    { NVODM_PORT('v'), 7, NvOdmGpioPinActiveState_Low, NULL },

    /* Panel 5 -- Harmony E1162 LVDS interface */
    { NVODM_PORT('b'), 2, NvOdmGpioPinActiveState_High, NULL },   // Enable (LVDS_SHTDN_N) (LO:OFF, HI:ON)
    { NVODM_PORT('w'), 0, NvOdmGpioPinActiveState_High, NULL },   // EN_VDD_BL
    { NVODM_PORT('b'), 5, NvOdmGpioPinActiveState_High, NULL },   // LCD_BL_EN
    { NVODM_PORT('c'), 6, NvOdmGpioPinActiveState_High, NULL },   // EN_VDD_PNL
    { NVODM_PORT('b'), 4, NvOdmGpioPinActiveState_High, NULL },   // LCD_BL_PWM
};

static const NvOdmGpioPinInfo s_hdmi[] =
{
    /* hdmi hot-plug interrupt pin */
    { NVODM_PORT('n'), 7, NvOdmGpioPinActiveState_High, NULL },    // HDMI HPD
};

static const NvOdmGpioPinInfo s_crt[] =
{
    /* crt hot-plug interrupt pin */
    { NVODM_PORT('x'), 2, NvOdmGpioPinActiveState_Low, NULL }, // VGA_DET#
};

static const NvOdmGpioPinInfo s_sdio[] = {
    {NVODM_PORT('i'), 5, NvOdmGpioPinActiveState_Low, NULL},    // Card Detect for SDIO instance 2
    /* High for WP and low for read/write */
    {NVODM_PORT('h'), 1, NvOdmGpioPinActiveState_High, NULL},    // Write Protect for SDIO instance 2 
};

static const NvOdmGpioPinInfo s_sdio3[] = {
    {NVODM_PORT('h'), 2, NvOdmGpioPinActiveState_Low, NULL},    // Card Detect for SDIO instance 3
    /* High for WP and low for read/write */

    {NVODM_PORT('h'), 3, NvOdmGpioPinActiveState_High, NULL},    // Write Protect for SDIO instance 3
};

static const NvOdmGpioPinInfo s_NandFlash[] = {
    {NVODM_PORT('c'), 7, NvOdmGpioPinActiveState_High, NULL} // Raw NAND WP_N
};

static const NvOdmGpioPinInfo s_spi_ethernet[] = {
    {NVODM_PORT('c'), 1, NvOdmGpioPinActiveState_Low, NULL}   // DBG_IRQ
};

static const NvOdmGpioPinInfo s_Bluetooth[] = {
    {NVODM_PORT('u'), 0, NvOdmGpioPinActiveState_Low, NULL}   // BT_RST#
};

static const NvOdmGpioPinInfo s_Wlan[] = {
    {NVODM_PORT('k'), 5, NvOdmGpioPinActiveState_Low, NULL},  // WF_PWDN#
    {NVODM_PORT('k'), 6, NvOdmGpioPinActiveState_Low, NULL}   // WF_RST#
};

static const NvOdmGpioPinInfo s_Power[] = {
    // lid open/close, High = Lid Closed
    {NVODM_PORT('u'), 5, NvOdmGpioPinActiveState_High, NULL},
    // power button
    {NVODM_PORT('v'), 2, NvOdmGpioPinActiveState_Low, NULL}
};

static const NvOdmGpioPinInfo s_EmbeddedController[] = {
    {NVODM_PORT('a'), 0, NvOdmGpioPinActiveState_Low, NULL}   // AP_WAKE#
};

// Gpio Pin key info
static const NvOdmGpioPinInfo s_GpioPinKeyInfo[] = {
    {0x1C, 10, NV_TRUE, NULL},
    {0xE04B, 10, NV_TRUE, NULL},
    {0xE04D, 10, NV_TRUE, NULL},
};

// Gpio based keypad
static const NvOdmGpioPinInfo s_GpioKeyBoard[] = {
    {NVODM_PORT('q'), 0, NvOdmGpioPinActiveState_Low, (void *)&s_GpioPinKeyInfo[0]},
    {NVODM_PORT('q'), 1, NvOdmGpioPinActiveState_Low, (void *)&s_GpioPinKeyInfo[1]},
    {NVODM_PORT('q'), 2, NvOdmGpioPinActiveState_Low, (void *)&s_GpioPinKeyInfo[2]},
};

static const NvOdmGpioPinInfo s_Battery[] = {
    // Low Battery
    {NVODM_PORT('w'), 3, NvOdmGpioPinActiveState_Low, NULL},
};
const NvOdmGpioPinInfo *NvOdmQueryGpioPinMap(NvOdmGpioPinGroup Group,
    NvU32 Instance, NvU32 *pCount)
{
    NvOdmBoardInfo BoardInfo;
    switch (Group)
    {
        case NvOdmGpioPinGroup_Display:
            *pCount = NVODM_ARRAY_SIZE(s_display);
            return s_display;

        case NvOdmGpioPinGroup_Hdmi:
            *pCount = NVODM_ARRAY_SIZE(s_hdmi);
            return s_hdmi;

        case NvOdmGpioPinGroup_Crt:
            *pCount = NVODM_ARRAY_SIZE(s_crt);
            return s_crt;

        case NvOdmGpioPinGroup_Sdio:
            if (Instance == 1)
            {
                *pCount = NVODM_ARRAY_SIZE(s_sdio);
                return s_sdio;
            }
            else if (Instance == 3)
            {
                *pCount = NVODM_ARRAY_SIZE(s_sdio3);
                return s_sdio3;
            }
            else
            {
                *pCount = 0;
                return NULL;
            }

        case NvOdmGpioPinGroup_NandFlash:
            *pCount = NVODM_ARRAY_SIZE(s_NandFlash);
            return s_NandFlash;

        case NvOdmGpioPinGroup_Bluetooth:
            *pCount = NVODM_ARRAY_SIZE(s_Bluetooth);
            return s_Bluetooth;

        case NvOdmGpioPinGroup_Wlan:
            *pCount = NVODM_ARRAY_SIZE(s_Wlan);
            return s_Wlan;

        case NvOdmGpioPinGroup_SpiEthernet:
            if (NvOdmQueryDownloadTransport() ==
                NvOdmDownloadTransport_SpiEthernet)
            {
                *pCount = NVODM_ARRAY_SIZE(s_spi_ethernet);
                return s_spi_ethernet;
            }
            else
            {
                *pCount = 0;
                return NULL;
            }
        
        case NvOdmGpioPinGroup_Vi:
            *pCount = NVODM_ARRAY_SIZE(s_vi);
            return s_vi;

        case NvOdmGpioPinGroup_Power:
            *pCount = NVODM_ARRAY_SIZE(s_Power);
            return s_Power;

        case NvOdmGpioPinGroup_EmbeddedController:
            *pCount = NVODM_ARRAY_SIZE(s_EmbeddedController);
            return s_EmbeddedController;

        case NvOdmGpioPinGroup_keypadMisc:
            if (NvOdmPeripheralGetBoardInfo(EEPROM_ID_E1206, &BoardInfo))
            {
                *pCount = NVODM_ARRAY_SIZE(s_GpioKeyBoard);
                return s_GpioKeyBoard;
            }
            *pCount = 0;
            return NULL;

        case NvOdmGpioPinGroup_Battery:
            *pCount = NVODM_ARRAY_SIZE(s_Battery);
            return s_Battery;

        default:
            *pCount = 0;
            return NULL;
    }
}

