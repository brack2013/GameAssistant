
LOCAL_PATH:= $(call my-dir)
OPENCV_PATH := /Users/Monect/Documents/OpenCV-android-sdk/sdk/native/jni

include $(CLEAR_VARS)
LOCAL_MODULE    := nonfree
LOCAL_SRC_FILES := libnonfree.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := opencv_java
LOCAL_SRC_FILES := libopencv_java.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

OpenCV_INSTALL_MODULES:=on

OPENCV_LIB_TYPE:=STATIC

include $(OPENCV_PATH)/OpenCV.mk


LOCAL_C_INCLUDES +=				\
    $(LOCAL_PATH)				\
    $(OPENCV_PATH)/include

LOCAL_SRC_FILES +=				\
    native_lib.cpp


LOCAL_MODULE := libnativelibjni
LOCAL_CFLAGS += -Werror -O3 -ffast-math
LOCAL_LDLIBS += -llog -ldl -lm
LOCAL_SHARED_LIBRARIES += nonfree
LOCAL_SHARED_LIBRARIES += opencv_java

include $(BUILD_SHARED_LIBRARY)
