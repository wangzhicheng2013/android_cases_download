LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := android_download_cases
LOCAL_SRC_FILES := main.cpp
LOCAL_LDLIBS := -pthread -lcurl -lssl -lcrypto -lc++
LOCAL_CFLAGS := -Wall -Werror -Wextra -Wno-unused-parameter -Wno-return-type -Wno-unused-variable -Wno-unused-function
include $(BUILD_EXECUTABLE)