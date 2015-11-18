################################################################################
#
# <t-base Sample Rot13 Trustlet
#
################################################################################


# output binary name without path or extension
OUTPUT_NAME := tlSampleRot13


#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------

TRUSTLET_UUID := 04010000000000000000000000000000
TRUSTLET_MEMTYPE := 2
TRUSTLET_NO_OF_THREADS := 1
TRUSTLET_SERVICE_TYPE := 3
TRUSTLET_KEYFILE := Locals/Build/pairVendorTltSig.pem
TRUSTLET_FLAGS := 0
TRUSTLET_INSTANCES := 1


#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS += \
    Locals/Code/public \
    $(COMP_PATH_DrSampleRot13)/Public/

### Add source code files for C++ compiler here
SRC_CPP += \
    Locals/Code/tlSampleRot13.c

### Add source code files for C compiler here
SRC_C += # nothing

### Add source code files for assembler here
SRC_S += # nothing

#-------------------------------------------------------------------------------
# DrRot13Sample related additions
#-------------------------------------------------------------------------------

ifeq ($(TOOLCHAIN),GNU)
    CUSTOMER_DRIVER_LIBS += \
        $(COMP_PATH_DrSampleRot13)/Bin/$(TOOLCHAIN)/drSampleRot13.lib
else
   CUSTOMER_DRIVER_LIBS += \
        $(COMP_PATH_DrSampleRot13)/Bin/drSampleRot13.lib
endif

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk

