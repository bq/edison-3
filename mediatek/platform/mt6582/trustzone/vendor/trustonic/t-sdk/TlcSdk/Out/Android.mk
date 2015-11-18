
# we provide the libraries with debug symbols. The symbols are stripped
# by the build system for the release version. Using generic build, as
# lib APIs are not platform dependent


LOCAL_PATH := $(call my-dir)


#-------------------------------------------------------------------------------
# Module: "MobiCoreDriver" library (libMcClient.so)
#-------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := MobiCoreDriver
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/Public \
                           $(LOCAL_PATH)/Public/GP \
                           $(COMP_PATH_TlSdk)/Public/MobiCore/inc
LOCAL_SRC_FILES := Bin/Generic/Debug/libMcClient.so
include $(PREBUILT_SHARED_LIBRARY)

