################################################################################
#
# <t-sdk Sample FOO Trustlet
#
################################################################################


# output binary name without path or extension
OUTPUT_NAME := tlfoo


#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------

TRUSTLET_UUID := 08010000000000000000000000000000
TRUSTLET_MEMTYPE := 2
TRUSTLET_NO_OF_THREADS := 1
TRUSTLET_SERVICE_TYPE := 3
TRUSTLET_KEYFILE := Locals/Build/pairVendorTltSig.pem
TRUSTLET_FLAGS := 0
TRUSTLET_INSTANCES := 10


#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS += \
    Locals/Code/public \
    $(DRFOO_DIR)/Public/

### Add source code files for C++ compiler here
SRC_CPP += \
    Locals/Code/tlfoo.c

### Add source code files for C compiler here
SRC_C += # nothing

### Add source code files for assembler here
SRC_S += # nothing

ifeq ($(TOOLCHAIN),GNU)
    CUSTOMER_DRIVER_LIBS += \
        $(DRFOO_OUT_DIR)/$(MTK_PROJECT)/$(TOOLCHAIN)/$(TEE_MODE)/drfoo.lib
else
   CUSTOMER_DRIVER_LIBS += \
        $(DRFOO_OUT_DIR)/$(MTK_PROJECT)/$(TOOLCHAIN)/$(TEE_MODE)/drfoo.lib
endif

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk

