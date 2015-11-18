# =============================================================================
#
# Makefile responsible for:
# - building a test binary - tlcFoo
# - building the TLC library - libtlcFooLib
#
# =============================================================================

# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)

# =============================================================================
# library tlcfooLib
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE	:= tlcfooLib

# Add your folders with header files here
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/inc \
	$(COMP_PATH_Tlfoo)/Public \
	$(COMP_PATH_TlSdk)/Public/MobiCore/inc

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= tlcfoo.cpp

# Need the MobiCore client library
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_STATIC_LIBRARY)


# =============================================================================
# binary tlcfoo
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE	:= tlcfoo

# Add your source files here
LOCAL_SRC_FILES	+= main.cpp

LOCAL_STATIC_LIBRARIES := tlcfooLib
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_EXECUTABLE)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
