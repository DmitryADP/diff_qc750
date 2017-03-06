#
# Copyright (c) 2011 - 2012 NVIDIA Corporation.  All rights reserved.
#
#define global variables

_avp_local_path := $(call my-dir)

_avp_local_cflags := -march=armv4t
_avp_local_cflags += -Os
_avp_local_cflags += -fno-unwind-tables
_avp_local_cflags += -fomit-frame-pointer
_avp_local_cflags += -ffunction-sections
_avp_local_cflags += -fdata-sections
ifeq ($(AVP_EXTERNAL_TOOLCHAIN),)
  _avp_local_cflags += -mno-thumb
else
  _avp_local_cflags += -mthumb-interwork
  _avp_local_cflags += -mthumb
endif

_avp_local_cflags += -DNVODM_BOARD_IS_FPGA=0
_avp_local_cflags += -DAES_KEYSCHED_LOCK_WAR_BUG_598910=0
_avp_local_cfalgs += -DSE_AES_KEYSCHED_READ_LOCK=1
_avp_local_cflags += -DNAND_SUPPORT=1
_avp_local_cflags += -DNO_BOOTROM=0
ifeq ($(HOST_OS),darwin)
_avp_local_cflags += -DNVML_BYPASS_PRINT=1
endif

_avp_local_c_includes += $(TARGET_OUT_HEADERS)
_avp_local_c_includes += $(TEGRA_TOP)/core/drivers/nvboot/t30/include
_avp_local_c_includes += $(TEGRA_TOP)/core/drivers/hwinc/t30
_avp_local_c_includes += $(TEGRA_TOP)/core/drivers/hwinc
_avp_local_c_includes += $(TEGRA_TOP)/core-private/drivers/hwinc
_avp_local_c_includes += $(TEGRA_TOP)/core/system/nvbct/t30
_avp_local_c_includes += $(TEGRA_TOP)/core/system/nvflash/nvml
_avp_local_c_includes += $(TEGRA_TOP)/core/system/nvflash/nvml/t30
_avp_local_c_includes += $(TEGRA_TOP)/core/utils/aes_keysched_lock
_avp_local_c_includes += $(TOP)/bionic/libc/arch-arm/include
_avp_local_c_includes += $(TOP)/bionic/libc/include
_avp_local_c_includes += $(TOP)/bionic/libc/kernel/common
_avp_local_c_includes += $(TOP)/bionic/libc/kernel/arch-arm
_avp_local_cflags += -Os
# Look for string.h in bionic libc include directory, in case we
# are using an Android prebuilt tool chain.  string.h recursively
# includes malloc.h, which would cause a declaration conflict with NvOS,
# so add a define in order to avoid recursive inclusion.
ifeq ($(AVP_EXTERNAL_TOOLCHAIN),)
  _avp_local_c_includes += $(TOP)/bionic/libc/include
  _avp_local_cflags += -D_MALLOC_H_
endif

# define miniloader common library to avoid ../ parent paths
LOCAL_PATH := $(_avp_local_path)

include $(NVIDIA_DEFAULTS)

LOCAL_MODULE := libnvml_common_t30

LOCAL_C_INCLUDES += $(_avp_local_c_includes)
LOCAL_CFLAGS     := $(_avp_local_cflags)

LOCAL_NO_DEFAULT_COMPILER_FLAGS := true

LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += nvml_server.c
LOCAL_SRC_FILES += nvml_nvboot.c
LOCAL_SRC_FILES += nvml_device.c

include $(NVIDIA_STATIC_AVP_LIBRARY)


# define miniloader executable
LOCAL_PATH := $(_avp_local_path)

include $(NVIDIA_DEFAULTS)

LOCAL_MODULE := nvflash_miniloader_t30

LOCAL_C_INCLUDES += $(_avp_local_c_includes)
LOCAL_CFLAGS     := $(_avp_local_cflags)
LOCAL_CFLAGS     += -DNO_MALLINFO

LOCAL_NO_DEFAULT_COMPILER_FLAGS := true

LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += t30/t30ml_utils.c
LOCAL_SRC_FILES += t30/nvml_aes.c
LOCAL_SRC_FILES += t30/nvml_hash.c

ifeq ($(filter t30, $(TARGET_TEGRA_VERSION)),)
LOCAL_CHIP := _t30
else
LOCAL_CHIP :=
endif

LOCAL_STATIC_LIBRARIES := libnv3p_avp
LOCAL_STATIC_LIBRARIES += libnvbct_avp
LOCAL_STATIC_LIBRARIES += libavpmain_avp
LOCAL_STATIC_LIBRARIES += libnvml_common_t30
LOCAL_STATIC_LIBRARIES += libnvavpuart
ifneq ($(HOST_OS),darwin)
LOCAL_STATIC_LIBRARIES += libnvodm_query_avp
endif
LOCAL_STATIC_LIBRARIES += libnvboot_clocks_avp$(LOCAL_CHIP)
LOCAL_STATIC_LIBRARIES += libnvboot_fuse_avp$(LOCAL_CHIP)
LOCAL_STATIC_LIBRARIES += libnvboot_nand_avp$(LOCAL_CHIP)
LOCAL_STATIC_LIBRARIES += libnvboot_snor_avp$(LOCAL_CHIP)
LOCAL_STATIC_LIBRARIES += libnvboot_pads_avp$(LOCAL_CHIP)
LOCAL_STATIC_LIBRARIES += libnvboot_reset_avp$(LOCAL_CHIP)
LOCAL_STATIC_LIBRARIES += libnvboot_sdram_avp$(LOCAL_CHIP)
LOCAL_STATIC_LIBRARIES += libnvboot_sdmmc_avp$(LOCAL_CHIP)
LOCAL_STATIC_LIBRARIES += libnvboot_spi_flash_avp$(LOCAL_CHIP)
LOCAL_STATIC_LIBRARIES += libnvboot_util_avp$(LOCAL_CHIP)
LOCAL_STATIC_LIBRARIES += libnvaes_keysched_lock_avp
LOCAL_STATIC_LIBRARIES += libnvseaes_keysched_lock_avp
LOCAL_STATIC_LIBRARIES += libnvboot_ahb$(LOCAL_CHIP)
LOCAL_STATIC_LIBRARIES += libnvboot_irom_patch$(LOCAL_CHIP)
ifeq ($(NV_TARGET_BOOTLOADER_PINMUX),kernel)
LOCAL_STATIC_LIBRARIES += libnvpinmux_avp
endif

# Potentially override the default compiler.  Froyo arm-eabi-4.4.0 toolchain
# does not produce valid code for ARMv4T ARM/Thumb interworking.
ifneq ($(AVP_EXTERNAL_TOOLCHAIN),)
  LOCAL_CC := $(AVP_EXTERNAL_TOOLCHAIN)gcc
endif

LOCAL_NVIDIA_LINK_SCRIPT_PATH := $(LOCAL_PATH)/t30

include $(NVIDIA_STATIC_AVP_EXECUTABLE)

# Preserve value of LOCAL_BUILT_MODULE path for generate header from it
_avp_local_built_module := $(LOCAL_BUILT_MODULE)

# miniloader header
LOCAL_PATH := $(_avp_local_path)

include $(NVIDIA_DEFAULTS)

LOCAL_MODULE := nvflash_miniloader_t30.h

LOCAL_SRC_FILES := $(_avp_local_built_module)

include $(NVIDIA_GENERATED_HEADER)

# variable cleanup
_avp_local_built_module :=
_avp_local_c_includes   :=
_avp_local_cflags       :=
_avp_local_path         :=

