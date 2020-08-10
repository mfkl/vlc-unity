APP_ABI := armeabi-v7a
# APP_ABI := armeabi-v7a arm64-v8a x86 x86_64
APP_PLATFORM := android-16
# TODO: ensure build with same NDK as libvlc to have the same libc++_shared.
APP_STL := c++_shared
APP_CPPFLAGS += -std=c++11
NDK_TOOLCHAIN_VERSION := clang
