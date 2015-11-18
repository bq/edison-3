################################################################################
#
# <t-base Async example Trustlet
#
################################################################################

### this is our home dir

TRUSTLET_DIR ?= Locals/Code

#-------------------------------------------------------------------------------
# Output
#-------------------------------------------------------------------------------
### output name without path or extension
OUTPUT_NAME := tlAsyncExample

### Mobiconvert parameters
## For more information about the different parameters, see the "MobiConvert User Manual"

## mandatory parameters

# the uuid( also used as output base file name)
TRUSTLET_UUID := 07080000000000000000000000000000
# --servicetype <num>
TRUSTLET_SERVICE_TYPE := 3 # 2: service provider trustlet; 3: system trustlet
# --keyfile <keyfile>
TRUSTLET_KEYFILE := $(TRUSTLET_DIR)/../Build/pairVendorTltSig.pem

## optional parameters
# --flags
TRUSTLET_FLAGS := 0 # 0: no flag; 1: permanent; 2: service has no WSM control interface; 3: both (permanent and service has not WSM control interface); 4: debuggable
# ---numberofinstances <num>
TRUSTLET_INSTANCES := 1 # min: 1; max: 16
# --numberofthreads <num>
TRUSTLET_NO_OF_THREADS := 1 # has to be 1 for Trustlets
# --memtype <type>
TRUSTLET_MEMTYPE := 2 # 0: internal memory prefered; 1: internal memory used; 2: external memory used
#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### A simple scatter file and set SCATTER_FILE.
### Uncomment this to use a custom scatter file $(OUTPUT_NAME).sct
# SCATTER_FILE := $(TRUSTLET_DIR)/$(OUTPUT_NAME).sct

### Add include path here
INCLUDE_DIRS += \
    $(TRUSTLET_DIR)/inc \
    $(TRUSTLET_DIR)/public \
    $(COMP_PATH_DrAsyncExample)/Public/

### Add source code files for C++ compiler here
SRC_CPP += # nothing


### Add source code files for C compiler here
SRC_C += \
    $(TRUSTLET_DIR)/tlTciHandler.c \
    $(TRUSTLET_DIR)/tlMain.c



### Add source code files for assembler here
SRC_S += # nothing


ifeq ($(TOOLCHAIN),GNU)
    CUSTOMER_DRIVER_LIBS += \
        $(COMP_PATH_DrAsyncExample)/Bin/$(TOOLCHAIN)/DrAsyncExample.lib
else
   CUSTOMER_DRIVER_LIBS += \
        $(COMP_PATH_DrAsyncExample)/Bin/DrAsyncExample.lib
endif

#

#-------------------------------------------------------------------------------
# use generic make file
include $(TLSDK_DIR_SRC)/trustlet.mk


