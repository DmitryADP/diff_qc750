# Generated by release script
LOCAL_PATH := $(call my-dir)
subdirs :=

productfiles := \
  $(TEGRA_TOP)/prebuilt/$(TARGET_PRODUCT)/Android.mk \
  

ifeq (,$(filter-out tegra%,$(TARGET_BOARD_PLATFORM)))
ifneq ($(HAVE_NVIDIA_PROP_SRC),false)
  include $(call all-named-subdir-makefiles,$(subdirs)) $(productfiles)
  include $(TEGRA_TOP)/core/modules.mk
endif
endif
