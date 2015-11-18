# =============================================================================
LOCAL_PATH	:= $(call my-dir)
MY_CLIENTLIB_PATH	:= $(LOCAL_PATH)


include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE	:= asyncExample

# Add your source files here (relative paths
LOCAL_SRC_FILES +=\
    asyncExample.c

LOCAL_C_INCLUDES +=\
    $(LOCAL_PATH)/inc \
    $(COMP_PATH_TlSdk)/Public/MobiCore/inc \
    $(COMP_PATH_TlAsyncExample)/Public

LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_EXECUTABLE)

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))

