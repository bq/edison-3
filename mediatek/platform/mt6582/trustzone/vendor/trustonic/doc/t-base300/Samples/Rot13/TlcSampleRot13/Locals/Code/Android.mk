# =============================================================================
#
# =============================================================================

# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)

# =============================================================================
# library tlcSampleRot13Lib
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE	:= tlcSampleRot13Lib

# Add your folders with header files here
LOCAL_C_INCLUDES += \
	$(COMP_PATH_TlSampleRot13)/Public \
	$(COMP_PATH_TlSdk)/Public/MobiCore/inc

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= tlcSampleRot13.cpp

# Need the MobiCore client library
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_STATIC_LIBRARY)

# =============================================================================
# binary tlcSampleRot13
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE	:= tlcSampleRot13

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= main.cpp

LOCAL_STATIC_LIBRARIES := tlcSampleRot13Lib
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_EXECUTABLE)

# =============================================================================
# JNI module: libTlcSampleRot13.so

include $(CLEAR_VARS)

LOCAL_MODULE	:= TlcSampleRot13

# Add new source files here
LOCAL_SRC_FILES	+= client-jni.cpp

LOCAL_STATIC_LIBRARIES := tlcSampleRot13Lib
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_SHARED_LIBRARY)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
