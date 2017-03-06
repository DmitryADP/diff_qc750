LOCAL_PATH := $(call my-dir)
include $(NVIDIA_DEFAULTS)

LOCAL_MODULE := libnvsystem_utils
LOCAL_NVIDIA_NO_COVERAGE := true
LOCAL_CFLAGS += -DENABLE_MULTIPARTITION=1
LOCAL_CFLAGS += -DNVODM_BOARD_IS_SIMULATION=0
LOCAL_SRC_FILES += nvsystem_utils.c
LOCAL_SRC_FILES += unsparseimg.c
LOCAL_SRC_FILES += crc32.c

LOCAL_NVIDIA_NO_EXTRA_WARNINGS := 1
include $(NVIDIA_STATIC_LIBRARY)
include $(NVIDIA_DEFAULTS)
LOCAL_MODULE := libnvsystem_utilshost
LOCAL_CFLAGS += -DENABLE_MULTIPARTITION=1
LOCAL_CFLAGS += -DNVODM_BOARD_IS_SIMULATION=1
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)
LOCAL_SRC_FILES += nvsystem_utils.c
LOCAL_SRC_FILES += unsparseimg.c
LOCAL_SRC_FILES += crc32.c
include $(NVIDIA_HOST_STATIC_LIBRARY)
