# Generated by release script
LOCAL_PATH := $(call my-dir)
subdirs := \
  ../../core/drivers/nvavpgpio \
  ../../core/drivers/nvpinmux \
  ../../core/drivers/nvavp/uart \
  ../../core/drivers/nvddk/disp \
  ../../core/drivers/nvddk/aes \
  ../../core/drivers/nvddk/snor \
  ../../core/drivers/nvddk/kbc \
  ../../core/drivers/nvddk/i2s \
  ../../core/drivers/nvddk/nand \
  ../../core/drivers/nvddk/blockdev \
  ../../core/drivers/nvddk/sdio \
  ../../core/drivers/nvddk/fuses/read \
  ../../core/drivers/nvddk/dap \
  ../../core/drivers/nvddk/spi_flash \
  ../../core/drivers/nvddk/se \
  ../../core/drivers/nvddk/spdif \
  ../../core/drivers/nvodm \
  ../../core/drivers/nvrm/nvrmkernel \
  ../../core/drivers/nvrm/graphics \
  ../../core/system/nv3p \
  ../../core/system/nv3pserver \
  ../../core/system/nvdiagnostics \
  ../../core/system/fastboot \
  ../../core/system/nvfs \
  ../../core/system/utils \
  ../../core/system/nvpartmgr \
  ../../core/system/nvcrypto \
  ../../core/system/microboot \
  ../../core/system/nvbct \
  ../../core/system/nvflash/app \
  ../../core/system/nvflash/lib \
  ../../core/system/nvfsmgr \
  ../../core/system/nvaboot \
  ../../core/system/nvbuildbct \
  ../../core/system/nvstormgr \
  ../../core/mobile_linux/daemons/nv_hciattach \
  ../../core/include \
  ../../core/utils/nvos \
  ../../core/utils/aes_keysched_lock \
  ../../core/utils/nvosutils \
  ../../core/utils/nvintr \
  ../../core/utils/nvappmain \
  ../../core/utils/nvapputil \
  ../../core/utils/tegrastats \
  ../../core/utils/nvfxmath \
  ../../core/utils/nvusbhost/libnvusbhost \
  ../../core/utils/nvreftrack \
  ../../core/utils/md5 \
  ../../3rdparty/libusb/libusb \
  ../../3rdparty/python-support-files \
  ../../3rdparty/khronos/conform/opengles2/conform/GTF_ES/utilityLibs/expat1.95.8 \
  ../../graphics-partner/android/hwcomposer \
  ../../graphics-partner/android/libgralloc \
  ../../multimedia-partner/openmax/ilclient \
  ../../multimedia-partner/utils/nvavp \
  ../../multimedia-partner/android/libaudio \
  ../../multimedia-partner/android/libstagefrighthw \
  ../../icera/ril/icera-ril \
  ../../icera/ril/apps/icera-switcher \
  ../../icera/ril/apps/DatacallWhitelister \
  ../../icera/ril/modules \
  ../../icera/ril/icera-util \
  ../../icera/apps/ModemErrorReporter \
  ../../tests-partner/hdcp/libhdcp_up \
  ../../tests-partner/hdcp/nvhdcp_test \
  ../../tests-partner/openmax/omxplayer2 \
  ../../tests-partner/wfd/nvcap_test \
  ../../tests-partner/wfd/NvwfdServiceTest \
  ../../tests-partner/wfd/NvwfdTest \
  ../../camera-partner/imager \
  ../../camera-partner/android/libnvomxcamera

productfiles := \
  $(TEGRA_TOP)/odm/${TARGET_PRODUCT}.mk \
  $(TEGRA_TOP)/prebuilt/$(TARGET_PRODUCT)/Android.mk

ifeq (,$(filter-out tegra%,$(TARGET_BOARD_PLATFORM)))
ifneq ($(HAVE_NVIDIA_PROP_SRC),false)
  include $(call all-named-subdir-makefiles,$(subdirs)) $(productfiles)
  include $(TEGRA_TOP)/core/modules.mk
endif
endif
