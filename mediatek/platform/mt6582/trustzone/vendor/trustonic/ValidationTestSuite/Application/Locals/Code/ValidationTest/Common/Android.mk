# =============================================================================
#
# Test utils for MobiCore Driver API
#
# =============================================================================

# This is not a separate module (yet).
# Only for inclusion by other modules.


# DO NOT MODIFY LOCAL SOURCES FILES INCLUSION HERE
# THE FOLLOWING MACROS WILL AUTOMATICALLY ADD ALL SOURCES FILES TO THE COMPILATION
# IF YOU WANT TO REMOVE A FILE FROM COMPILATION, PLEASE CUSTOMIZE YOUR PTD

MY_SOURCES := $(wildcard $(call my-dir)/*.c $(call my-dir)/*.cpp)
LOCAL_SRC_FILES += $(MY_SOURCES:$(LOCAL_PATH)/%=%)
