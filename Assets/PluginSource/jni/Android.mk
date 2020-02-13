LOCAL_PATH := $(call my-dir)
SRC_DIR := ../

include $(CLEAR_VARS)

LOCAL_MODULE := LibVLC
LOCAL_EXPORT_C_INCLUDES := $(SRC_DIR)/include
LOCAL_SRC_FILES := ../../../libvlc.so

include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := VLCUnityPlugin

LOCAL_LDLIBS := -llog
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES += $(SRC_DIR)/RenderAPI.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/RenderingPlugin.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/RenderAPI_Android.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/Log.cpp

# LibVLC
LOCAL_SHARED_LIBRARIES = LibVLC 

# OpenGL ES
LOCAL_SRC_FILES += $(SRC_DIR)/RenderAPI_OpenGLBase.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/RenderAPI_OpenGLEGL.cpp
LOCAL_LDLIBS += -lGLESv2
LOCAL_CPPFLAGS += -DSUPPORT_OPENGL_ES=1 -DUNITY_ANDROID=1

# Vulkan (optional)
# LOCAL_SRC_FILES += $(SRC_DIR)/RenderAPI_Vulkan.cpp
# LOCAL_C_INCLUDES += $(NDK_ROOT)/sources/third_party/vulkan/src/include
# LOCAL_CPPFLAGS += -DSUPPORT_VULKAN=1

# build
include $(BUILD_SHARED_LIBRARY)
