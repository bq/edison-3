# =============================================================================
#
# Module: CppUTest library (libCppUTest.a) to be imported by TLC's
#
# =============================================================================

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := CppUTest

# Printing debug messages to logcat
LOCAL_EXPORT_LDLIBS := -llog

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/Public

# Providing the library with debug symbols.
# Symbols are stripped by the build system for the release version.
LOCAL_SRC_FILES := Bin/Debug/libCppUTest.a

include $(PREBUILT_STATIC_LIBRARY)
