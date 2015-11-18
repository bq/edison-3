################################################################################
#
# <t-base Makefile Template
#
################################################################################

### this is our home dir

DRIVER_DIR ?= Locals/Code
DRLIB_DIR  := $(DRIVER_DIR)/api

#-------------------------------------------------------------------------------
# Output
#-------------------------------------------------------------------------------
### output name without path or extension
OUTPUT_NAME := DrAsyncExample

### Mobiconvert parameters
DRIVER_UUID := 07090000000000000000000000000000
DRIVER_MEMTYPE := 2 # 0: iram preferred; 1: iram; 2: eram
DRIVER_NO_OF_THREADS := 4 # min =1; max =8
DRIVER_SERVICE_TYPE := 1 # 1: driver; 2: service provider trustlet; 3: system trustlet
DRIVER_KEYFILE := $(DRIVER_DIR)/../Build/pairVendorTltSig.pem
DRIVER_FLAGS:= 0 # 0: no flag; 1: permanent; 2: service has no WSM control interface; 3: both (permanent and service has not WSM control interface)
DRIVER_VENDOR_ID :=0 # Trustonic
DRIVER_NUMBER := 0x0201 # DrAsyncExample
DRIVER_ID := $$(($(DRIVER_VENDOR_ID)<<16|$(DRIVER_NUMBER)))
DRIVER_INTERFACE_VERSION_MAJOR=1
DRIVER_INTERFACE_VERSION_MINOR=0
DRIVER_INTERFACE_VERSION := $(DRIVER_INTERFACE_VERSION_MAJOR).$(DRIVER_INTERFACE_VERSION_MINOR)

#-------------------------------------------------------------------------------
# DCI handler and trustlet APIs are enabled as default
#-------------------------------------------------------------------------------
USE_DCI_HANDLER ?= NO
USE_TL_API ?= YES

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### A simple scatter file and set SCATTER_FILE.
### Uncomment this to use a custom scatter file $(OUTPUT_NAME).sct
# SCATTER_FILE := $(DRIVER_DIR)/$(OUTPUT_NAME).sct

### Add include path here
INCLUDE_DIRS += \
    $(DRIVER_DIR)/inc \
    $(DRIVER_DIR)/public

### Add library source code files for C compiler here
ifeq ($(USE_TL_API),YES)
SRC_LIB_C += \
    $(DRLIB_DIR)/tlAsyncExampleDriverApi.c
else
SRC_LIB_C +=  # nothing
endif

### Add source code files for C++ compiler here
SRC_CPP += # nothing

### Add source code files for C compiler here
SRC_C += \
    $(DRIVER_DIR)/drMain.c \
    $(DRIVER_DIR)/drExcHandler.c \
    $(DRIVER_DIR)/drIpcHandler.c \
    $(DRIVER_DIR)/drSmgmt.c \
    $(DRIVER_DIR)/drUtils.c \
  	$(DRIVER_DIR)/drTimerIsr.c \
  	
ifeq ($(USE_DCI_HANDLER),YES)
    SRC_C += $(DRIVER_DIR)/drDciHandler.c
endif


ARMCC_COMPILATION_FLAGS := # init as empty

### Add source code files for assembler here
SRC_ASM += # nothing

ifneq (,$(filter $(PLATFORM),ARM_VE_A9X4_STD EXYNOS_4X12_STD EXYNOS_5250_STD))
    ifneq (,$(filter $(PLATFORM),ARM_VE_A9X4_STD))
        ifeq ($(TOOLCHAIN),GNU)
            SRC_ASM += $(DRIVER_DIR)/platform/$(ARM_SUBDIR)/timer_gnu.S 
        else
            SRC_ASM += $(DRIVER_DIR)/platform/$(ARM_SUBDIR)/timer_arm.S 
        endif
    else
        SRC_C += $(DRIVER_DIR)/platform/$(ARM_SUBDIR)/timer.c
    endif
else
    ifeq ($(PLATFORM),)
        $(info - PLATFORM  is  not  set,  default  is : ARM_V7A_STD)
        PLATFORM ?= ARM_V7A_STD
        ifeq ($(TOOLCHAIN),GNU)
            SRC_ASM += $(DRIVER_DIR)/platform/$(ARM_SUBDIR)/timer_gnu.S 
        else
            SRC_ASM += $(DRIVER_DIR)/platform/$(ARM_SUBDIR)/timer_arm.S 
        endif
    else
        ARMCC_COMPILATION_FLAGS += -DNO_HW_TIMER
    endif
endif

#Load value for timer (used to show how preprocessor works)
ARMCC_COMPILATION_FLAGS += -DTIMER_LOAD_VALUE=1000000


### Add additional compilation flags
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
DRSDK_DIR_SRC ?= $(DRSDK_DIR)
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(DRSDK_DIR_SRC)/driver.mk

