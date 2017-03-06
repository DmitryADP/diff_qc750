LOCAL_PATH := $(call my-dir)
include $(NVIDIA_DEFAULTS)

LOCAL_MODULE := libnvodm_keyboard
LOCAL_NVIDIA_NO_COVERAGE := true
LOCAL_SRC_FILES += nvodm_keyboard.c
LOCAL_SRC_FILES += nvodm_keyboard_ec.c
LOCAL_SRC_FILES += nvodm_keyboard_gpio.c

include $(NVIDIA_STATIC_LIBRARY)

