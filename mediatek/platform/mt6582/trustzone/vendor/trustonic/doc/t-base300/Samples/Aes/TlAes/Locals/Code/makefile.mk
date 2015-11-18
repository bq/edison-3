################################################################################
#
# <t-sdk Sample AES Trustlet
#
################################################################################


# output binary name without path or extension
OUTPUT_NAME := tlAes


#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------

TRUSTLET_UUID := 07020000000000000000000000000000
TRUSTLET_MEMTYPE := 2
TRUSTLET_NO_OF_THREADS := 1
TRUSTLET_SERVICE_TYPE := 2
TRUSTLET_KEYFILE := Locals/Build/keySpTl.xml
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
    Locals/Code/tlAes.c

### Add source code files for C compiler here
SRC_C += # nothing

### Add source code files for assembler here
SRC_S += # nothing

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk

