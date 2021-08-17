LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  vgg_sample.cpp 

LOCAL_C_INCLUDES := \
  frameworks/nn/include \
  frameworks/nn/include/vx_ext \
  frameworks/nn/include/libnn-v2

LOCAL_CPP_EXTENSION := .cpp
LOCAL_SHARED_LIBRARIES := \
  libopenvx \
  libopenvx-nn \
  libnn \
  libsoft-nn \
  liblog \
  liblombo_malloc
  
#LOCAL_MODULE_TAGS := optional
# 注释此行如果你希望保留符合和调试信息
LOCAL_STRIP_MODULE := false

LOCAL_MODULE:= profile-vgg

include $(BUILD_EXECUTABLE)















