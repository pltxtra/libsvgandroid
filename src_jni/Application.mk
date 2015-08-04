APP_BUILD_SCRIPT := $(call my-dir)/Android.mk
APP_STL := gnustl_static
APP_CPPFLAGS += -fexceptions
APP_CPPFLAGS += -frtti
APP_ABI := armeabi
NDK_TOOLCHAIN_VERSION=4.9
