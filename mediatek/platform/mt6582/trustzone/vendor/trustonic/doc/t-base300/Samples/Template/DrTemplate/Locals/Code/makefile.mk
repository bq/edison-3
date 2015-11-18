################################################################################
#
# <t-base Makefile Template
#
################################################################################

# output binary name without path or extension
OUTPUT_NAME := drTemplate


#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------
DRIVER_UUID := 07050000000000000000000000000000
DRIVER_MEMTYPE := 1 # 0: iram preferred; 1: iram; 2: eram
DRIVER_NO_OF_THREADS := 3 # min =1; max =8
DRIVER_SERVICE_TYPE := 1 # 1: driver; 2: service provider trustlet; 3: system trustlet
DRIVER_KEYFILE := Locals/Build/pairVendorTltSig.pem
DRIVER_FLAGS:= 0 # 0: no flag; 1: permanent; 2: service has no WSM control interface; 3: both (permanent and service has not WSM control interface)
DRIVER_VENDOR_ID :=0 # Trustonic
DRIVER_NUMBER := 0x0200 # DrTemplate
DRIVER_ID := $$(($(DRIVER_VENDOR_ID)<<16|$(DRIVER_NUMBER)))
DRIVER_INTERFACE_VERSION_MAJOR=1
DRIVER_INTERFACE_VERSION_MINOR=0
DRIVER_INTERFACE_VERSION := $(DRIVER_INTERFACE_VERSION_MAJOR).$(DRIVER_INTERFACE_VERSION_MINOR)

#-------------------------------------------------------------------------------
# DCI handler and trustlet APIs are enabled as default
#-------------------------------------------------------------------------------
USE_DCI_HANDLER ?= YES
USE_TL_API ?= YES


#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------


### Add include path here
INCLUDE_DIRS += \
    Locals/Code/inc \
    Locals/Code/public

### Add library source code files for C compiler here
ifeq ($(USE_TL_API),YES)
SRC_LIB_C += \
    Locals/Code/tlDriverApi.c
else
SRC_LIB_C +=  # nothing
endif

### Add source code files for C++ compiler here
SRC_CPP += # nothing

### Add source code files for C compiler here
SRC_C += \
    Locals/Code/drMain.c \
    Locals/Code/drExcHandler.c \
    Locals/Code/drIpcHandler.c \
    Locals/Code/drSmgmt.c \
    Locals/Code/drUtils.c

ifeq ($(USE_DCI_HANDLER),YES)
    SRC_C += Locals/Code/drDciHandler.c
endif

### Add source code files for assembler here
SRC_S += # nothing

ifeq ($(USE_TL_API),YES)
    ARMCC_COMPILATION_FLAGS += -DDR_FEATURE_TL_API
endif

ifeq ($(USE_DCI_HANDLER),YES)
    ARMCC_COMPILATION_FLAGS += -DDR_FEATURE_DCI_HANDLER
endif

ARMCC_COMPILATION_FLAGS += -DDRIVER_ID=$(DRIVER_ID)
ARMCC_COMPILATION_FLAGS += -DDRIVER_VERSION_MAJOR=$(DRIVER_INTERFACE_VERSION_MAJOR)
ARMCC_COMPILATION_FLAGS += -DDRIVER_VERSION_MINOR=$(DRIVER_INTERFACE_VERSION_MINOR)

#-------------------------------------------------------------------------------
# use generic make file
PLATFORM ?= ARM_V7A_STD
DRIVER_DIR ?= Locals/Code
DRLIB_DIR := $(DRIVER_DIR)/api
DRSDK_DIR_SRC ?= $(DRSDK_DIR)
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(DRSDK_DIR)/driver.mk

