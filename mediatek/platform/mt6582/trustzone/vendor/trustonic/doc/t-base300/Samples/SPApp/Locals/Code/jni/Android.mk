# =============================================================================
#
# Module: libSPApp.so
#
# =============================================================================
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libSPApp
LOCAL_CFLAGS += -DLOG_TAG=\"TbSPApp\"
LOCAL_C_INCLUDES += $(TLSDK_DIR)/Public/MobiCore/inc \
					$(MOBICOREDRIVERLIB_DIR)/Public
LOCAL_SRC_FILES	+= spapp.cpp

include $(LOGWRAPPER_DIR)/Android.mk
include $(BUILD_SHARED_LIBRARY)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
$(call import-add-path,/)
$(call import-module,$(MOBICOREDRIVERLIB_DIR))
