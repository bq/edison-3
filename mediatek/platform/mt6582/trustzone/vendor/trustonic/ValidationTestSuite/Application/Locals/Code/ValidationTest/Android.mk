# =============================================================================
#
# Module: MobiCore Trustlet Connector for Integration Tests
#
# =============================================================================

LOCAL_PATH	:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ValidationTestSuite

# folders with header files
LOCAL_C_INCLUDES += \
	$(COMP_PATH_TlIntegrationTest)/Public \
	$(COMP_PATH_DrIntegrationTest)/Public \
	$(COMP_PATH_MobiCore)/inc \
	$(COMP_PATH_MobiCoreDriverMod)/Public

# c and c++ source files

# DO NOT MODIFY LOCAL SOURCES FILES INCLUSION HERE
# THE FOLLOWING MACROS WILL AUTOMATICALLY ADD ALL SOURCES FILES TO THE COMPILATION
# IF YOU WANT TO REMOVE A FILE FROM COMPILATION, PLEASE CUSTOMIZE YOUR PTD

MY_SOURCES := $(wildcard $(LOCAL_PATH)/*.c $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES += $(MY_SOURCES:$(LOCAL_PATH)/%=%)

# Including all Android.mk files from subdirectories
include $(filter-out $(LOCAL_PATH)/Android.mk,$(shell find $(LOCAL_PATH)/ -type f -name Android.mk))


LOCAL_STATIC_LIBRARIES	:= CppUTest
LOCAL_SHARED_LIBRARIES	:= MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_EXECUTABLE)
