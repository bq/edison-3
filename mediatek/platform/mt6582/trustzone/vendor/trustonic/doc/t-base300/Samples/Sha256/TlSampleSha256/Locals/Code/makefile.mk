################################################################################
#
# <t-sdk Sample Sha256 Trustlet
#
################################################################################


# output binary name without path or extension
OUTPUT_NAME := tlSampleSha256


#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------

TRUSTLET_UUID := 06010000000000000000000000000000
TRUSTLET_MEMTYPE := 2
TRUSTLET_NO_OF_THREADS := 1
TRUSTLET_SERVICE_TYPE := 2 # 2: service provider trustlet; 3: system trustlet
TRUSTLET_KEYFILE := Locals/Build/keySpTl.xml            #only valid for service provider trustlets.
TRUSTLET_FLAGS := 4
TRUSTLET_INSTANCES := 16


#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS += \
    Locals/Code/public

### Add source code files for C++ compiler here
SRC_CPP += \
    Locals/Code/tlSampleSha256.c

### Add source code files for C compiler here
SRC_C += # nothing

### Add source code files for assembler here
SRC_S += # nothing

CUSTOMER_DRIVER_LIBS += # nothing

ARMCC_COMPILATION_FLAGS += # nothing

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk

