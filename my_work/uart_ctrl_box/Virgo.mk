LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= libuart_ctrl
#LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	src/crc16.c \
	src/uart.c \
	src/ctrl_box.c \
    src/uart_test.c

LOCAL_C_INCLUDES := \
    apps/voyage-demo/uart_ctrl_box/include
	
LOCAL_LDFLAGS := -lpthread -lm -lts -lz -lfreetype

include $(BUILD_SHARED_LIBRARY)



###########################################



include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
	src/crc16.c \
	src/uart.c \
	src/ctrl_box.c \
    src/uart_test.c

LOCAL_MODULE:= uart_ctrl_test
LOCAL_MODULE_TAGS := optional


LOCAL_C_INCLUDES := \
	apps/voyage-demo/uart_ctrl_box/include
	
LOCAL_LDFLAGS := -lpthread -lm -lts -lz -lfreetype

LOCAL_SHARED_LIBRARIES := \
	libuart_ctrl

include $(BUILD_EXECUTABLE)
