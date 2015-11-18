# =============================================================================
#
# Makefile responsible for:
# - building the Java JNI wrapper - TlcPinpad.so
# - building a test binary - pinpadTest
# - building the TLC library - libtlcPinpad.a (used by test binary + Java lib)
#
# =============================================================================

# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)
# =============================================================================
# TLC Pinpad library - libtlcPinpad.a

include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE	:= tlcPinpad

# Add your folders with header files here
LOCAL_C_INCLUDES += \
	$(COMP_PATH_TlSamplePinpad)/Public

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= \
	tlcPinpad.cpp

# Need the MobiCore client library
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_STATIC_LIBRARY)


# =============================================================================
# JNI module: TlcPinpad.so

include $(CLEAR_VARS)

LOCAL_MODULE	:= TlcPinpad

# Add new source files here
LOCAL_SRC_FILES	+= jni/tlcPinpadJni.cpp

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/pictures \
	$(COMP_PATH_TlSamplePinpad)/Public

LOCAL_STATIC_LIBRARIES := tlcPinpad
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_SHARED_LIBRARY)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
