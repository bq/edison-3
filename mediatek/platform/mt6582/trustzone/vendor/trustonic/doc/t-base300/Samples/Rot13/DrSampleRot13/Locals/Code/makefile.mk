################################################################################
#
# <t-sdk Rot13 Driver
#
################################################################################


# output binary name without path or extension
OUTPUT_NAME := drSampleRot13


#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------
DRIVER_UUID := 020a0000000000000000000000000000
DRIVER_MEMTYPE := 2 # 0: iram preferred; 1: iram; 2: eram
DRIVER_NO_OF_THREADS := 3 # min =1; max =8
DRIVER_SERVICE_TYPE := 1 # 1: driver; 2: service provider trustlet; 3: system trustlet
DRIVER_KEYFILE := Locals/Build/pairVendorTltSig.pem
DRIVER_FLAGS:= 0 # 0: no flag; 1: permanent; 2: service has no WSM control interface; 3: both (permanent and service has not WSM control interface)
DRIVER_VENDOR_ID :=0 # Trustonic
DRIVER_NUMBER := 0x8001 # DrSampleRot13
DRIVER_ID := $$(($(DRIVER_VENDOR_ID)<<16|$(DRIVER_NUMBER)))
DRIVER_INTERFACE_VERSION_MAJOR=1
DRIVER_INTERFACE_VERSION_MINOR=0
DRIVER_INTERFACE_VERSION := $(DRIVER_INTERFACE_VERSION_MAJOR).$(DRIVER_INTERFACE_VERSION_MINOR)

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS += \
    Locals/Code \
    Locals/Code/inc \
    Locals/Code/public \
    Locals/Code/Api

### Add library source code files for C compiler here
SRC_LIB_C += \
    Locals/Code/TlApiRot13.c

### Add source code files for C++ compiler here
SRC_CPP += \
    Locals/Code/drSampleRot13Main.c \
    Locals/Code/drSampleRot13Ipch.c \
    Locals/Code/drSampleRot13Exch.c \
    Locals/Code/drSampleRot13Utils.c \
    Locals/Code/Rot13Sample_hw_hal.c

### Add source code files for C compiler here
SRC_C += #nothing

### Add source code files for assembler here
SRC_ASM += # nothing

ARMCC_COMPILATION_FLAGS += -DDRIVER_ID=$(DRIVER_ID)
ARMCC_COMPILATION_FLAGS += -DDRIVER_VERSION_MAJOR=$(DRIVER_INTERFACE_VERSION_MAJOR)
ARMCC_COMPILATION_FLAGS += -DDRIVER_VERSION_MINOR=$(DRIVER_INTERFACE_VERSION_MINOR)

#-------------------------------------------------------------------------------
# use generic make file
PLATFORM ?= ARM_V7A_STD
DRIVER_DIR ?= Locals/Code
DRLIB_DIR := $(DRIVER_DIR)/Api
DRSDK_DIR_SRC ?= $(DRSDK_DIR)
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(DRSDK_DIR)/driver.mk

