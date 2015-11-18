# =============================================================================
#
# Makefile responsible for:
# - building the Java JNI wrapper - libRot13JavaClient.so
# - building the CA binary - caSampleRot13
# - building the CA library - libcaSampleRot13.a (used by test binary + Java lib)
#
# =============================================================================

# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)

# =============================================================================
# CA Rot13 library - libcaSampleRot13.a

include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE	:= caSampleRot13Lib

# Add your folders with header files hereample
LOCAL_C_INCLUDES += \
    $(COMP_PATH_TASampleRot13)/Public \

LOCAL_CFLAGS += -DTBASE_API_LEVEL=3

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= caSampleRot13.cpp

# Need the MobiCore client library
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_STATIC_LIBRARY)


# =============================================================================
# JNI module: libRot13JavaClient.so

include $(CLEAR_VARS)

LOCAL_MODULE	:= Rot13JavaClient

# Add new source files here
LOCAL_SRC_FILES	+= client-jni.cpp

LOCAL_STATIC_LIBRARIES := caSampleRot13Lib
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_SHARED_LIBRARY)


# =============================================================================
# Rot13 test binary

include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE	:= caSampleRot13

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= main.cpp

LOCAL_STATIC_LIBRARIES := caSampleRot13Lib
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_EXECUTABLE)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
