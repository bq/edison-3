################################################################################
#
# DRM Driver Makefile
#
################################################################################

DRIVER_DIR ?= ../Code
DRLIB_DIR  := $(DRIVER_DIR)/api

# Define default platform
# This sets compiler option and selects the DrApi binary.
# Also common MobiCore defines are set.
PLATFORM ?= ARM_VE_A9X4_STD
 

#-------------------------------------------------------------------------------
# Output
#-------------------------------------------------------------------------------
### output name without path or extension
OUTPUT_NAME := tlDrTplayDrm

### Mobiconvert parameters
DRIVER_UUID 			 := 070b0000000000000000000000000000
DRIVER_MEMTYPE 			 := 2 # 0: iram preferred; 1: iram; 2: eram
DRIVER_NO_OF_THREADS 	 := 3 # min =1; max =8
DRIVER_SERVICE_TYPE 	 := 1 # 1: driver; 2: service provider trustlet; 3: system trustlet
DRIVER_KEYFILE 			 := $(DRIVER_DIR)/../Build/pairVendorTltSig.pem
DRIVER_FLAGS			 := 0 # 0: no flag; 1: permanent; 2: service has no WSM control interface; 3: both (permanent and service has not WSM control interface)
DRIVER_ID 				 := 1536 # >100
DRIVER_INTERFACE_VERSION := 1.0


#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### A simple scatter file and set SCATTER_FILE.
### Uncomment this to use a custom scatter file $(OUTPUT_NAME).sct
# SCATTER_FILE := $(DRIVER_DIR)/$(OUTPUT_NAME).sct

### Add include path here
INCLUDE_DIRS += \
    $(DRIVER_DIR)/inc \
    $(DRIVER_DIR)/public \
    $(TLSDK_DIR_SRC)/Public \
    $(TLSDK_DIR_SRC)/Public/MobiCore/inc

### Add library source code files for C compiler here
SRC_LIB_C += \
    $(DRLIB_DIR)/tlTplayDriverApi.c

### Add source code files for C++ compiler here
SRC_CPP += # nothing

### Add source code files for C compiler here
SRC_C += \
    $(DRIVER_DIR)/drTplayMain.c \
    $(DRIVER_DIR)/drTplayExcHandler.c \
    $(DRIVER_DIR)/drTplayIpcHandler.c \
    $(DRIVER_DIR)/drTplayUtils.c

### Add additional compilation flags
#ARMCC_COMPILATION_FLAGS := -DDEBUG_NO_SECURITY_CHECK

ARMCC_COMPILATION_FLAGS += -DDRIVER_ID=$(DRIVER_ID)
#-------------------------------------------------------------------------------
# use generic make file
include $(DRSDK_DIR_SRC)/driver.mk

