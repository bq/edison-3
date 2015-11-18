################################################################################
#
# <t-sdk Sample Rot13 Trusted Application
#
################################################################################


# output binary name without path or extension
OUTPUT_NAME := taSampleRot13
GP_ENTRYPOINTS := Y


#-------------------------------------------------------------------------------
# t-base-convert parameters, see manual for details
#-------------------------------------------------------------------------------

TA_UUID := 08010000000000000000000000000000
TA_MEMTYPE := 2
TA_NO_OF_THREADS := 1
TA_SERVICE_TYPE := 2
TA_KEYFILE := Locals/Build/keySpTl.xml
TA_FLAGS := 0
TA_INSTANCES := 1

#-------------------------------------------------------------------------------

TBASE_API_LEVEL := 3

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS += \
    Locals/Code/public \
    Locals/Code/include \

### Add source code files for C++ compiler here
SRC_CPP += \
    Locals/Code/rot13_service.c \

### Add source code files for C compiler here
SRC_C += # nothing

### Add source code files for assembler here
SRC_S += # nothing

#-------------------------------------------------------------------------------
# use generic make file
TRUSTED_APP_DIR ?= Locals/Code
TASDK_DIR_SRC ?= $(TASDK_DIR)
include $(TASDK_DIR)/trusted_application.mk
