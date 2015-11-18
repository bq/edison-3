 #/// @defgroup TEST_MM    MemoryManagement
 #/// @ingroup  SUBJECT
 #/// These tests verify the correct working of allocations and deallocations in NWd components,<br> as well as correct setup of shared memory with SWd.
 #///
 #///      @defgroup TEST_MM_TCI   Example Sub Group - TCI Buffer Tests
 #///      @ingroup  TEST_MM



# =============================================================================
#
# Basic Test Cases for MobiCore Driver API
#
# =============================================================================

# This is not a separate module (yet).
# Only for inclusion by other modules.

# DO NOT MODIFY LOCAL SOURCES FILES INCLUSION HERE
# THE FOLLOWING MACROS WILL AUTOMATICALLY ADD ALL SOURCES FILES TO THE COMPILATION
# IF YOU WANT TO REMOVE A FILE FROM COMPILATION, PLEASE CUSTOMIZE YOUR PTD

MY_SOURCES := $(wildcard $(call my-dir)/*.c $(call my-dir)/*.cpp)
LOCAL_SRC_FILES += $(MY_SOURCES:$(LOCAL_PATH)/%=%)
