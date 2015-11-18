# =============================================================================
LOCAL_PATH	:= $(call my-dir)

include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE	:= TlcPlay

# Add your source files here (relative paths
LOCAL_SRC_FILES +=\
    TlcPlay.c

LOCAL_C_INCLUDES +=\
    $(COMP_PATH_DrTplayDrmSkeleton)/Public \
    $(COMP_PATH_TlPlay)/Public \
    $(COMP_PATH_TlSdk)/Public/MobiCore/inc

LOCAL_SHARED_LIBRARIES := \
    MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk
include $(BUILD_EXECUTABLE)

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))

