# =============================================================================
#
# =============================================================================

# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)

# =============================================================================
# library tlcSampleAesLib
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE	:= tlcSampleAesLib

# Add your folders with header files here
LOCAL_C_INCLUDES += \
	$(COMP_PATH_TlAes)/Public \
	$(COMP_PATH_TlSdk)/Public/MobiCore/inc

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= tlcSampleAes.cpp

# Need the MobiCore client library
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_STATIC_LIBRARY)

# =============================================================================
# binary tlcSampleAes
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE	:= tlcSampleAes

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= main.cpp

LOCAL_STATIC_LIBRARIES := tlcSampleAesLib
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_EXECUTABLE)

# =============================================================================
# JNI module: libTlcSampleAes.so

include $(CLEAR_VARS)

LOCAL_MODULE	:= TlcSampleAes

# Add new source files here
LOCAL_SRC_FILES	+= client-jni.cpp

LOCAL_STATIC_LIBRARIES := tlcSampleAesLib
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_SHARED_LIBRARY)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
