# ************************************************************************************************
# Environment
$(info ******************************************)
$(info Trusted Application Build)
$(info ******************************************)

# TEE_MODE : Debug, Release
ifeq ($(TEE_MODE),)
$(info - MODE  is  not  set,  default  is : Debug)
TEE_MODE ?= Debug
endif

# TOOLCHAIN : ARM, GNU
ifeq ($(TOOLCHAIN),)
$(info - TOOLCHAIN is not set, default is : ARM)
TOOLCHAIN ?= GNU
endif

# GP ENTRY : tlMain, GP Entrypoints
ifeq ($(GP_ENTRYPOINTS),)
$(info - GP_ENTRYPOINTS is not set, default is : N)
GP_ENTRYPOINTS ?= N
endif

# TA_INTERFACE_VERSION : major.minor
ifeq ($(TA_INTERFACE_VERSION),)
$(info - TA_INTERFACE_VERSION  is  not  set,  default  is : 0.0)
TA_INTERFACE_VERSION ?= 0.0
endif


# TBASE_API_LEVEL
ifeq ($(TBASE_API_LEVEL),)
$(info - TBASE_API_LEVEL  is  not  set,  default  is : 2)
TBASE_API_LEVEL ?= 2
endif

$(info ******************************************)
$(info MODE      : $(TEE_MODE))
$(info TOOLCHAIN : $(TOOLCHAIN))
$(info GP-ENTRIES          : $(GP_ENTRYPOINTS))
$(info TA_INTERFACE_VERSION: $(TA_INTERFACE_VERSION))
$(info TBASE_API_LEVEL     : $(TBASE_API_LEVEL))
$(info ******************************************)

# Check if variables are correctly set
ifneq ($(TEE_MODE),Debug)
   ifneq ($(TEE_MODE),Release)
      $(error ERROR : MODE value is not correct : $(TEE_MODE))
   endif
endif
ifneq ($(TOOLCHAIN),GNU)
   ifneq ($(TOOLCHAIN),ARM)
      $(error ERROR : TOOLCHAIN value is not correct : $(TOOLCHAIN))
   endif
endif
ifneq ($(GP_ENTRYPOINTS),N)
   ifneq ($(GP_ENTRYPOINTS),Y)
      $(error ERROR : GP_ENTRYPOINTS value is not correct : $(GP))
   endif
endif


# Accept old variable names
TRUSTED_APP_DIR ?= $(TRUSTLET_DIR)

TA_UUID ?= $(TRUSTLET_UUID)
TA_UUID := $(shell echo $(TA_UUID) | tr A-Z a-z)
TA_MEMTYPE ?= $(TRUSTLET_MEMTYPE)
TA_NO_OF_THREADS ?= $(TRUSTLET_NO_OF_THREADS)
TA_SERVICE_TYPE ?= $(TRUSTLET_SERVICE_TYPE)
TA_KEYFILE ?= $(TRUSTLET_KEYFILE)
TA_FLAGS ?= $(TRUSTLET_FLAGS)
TA_INSTANCES ?= $(TRUSTLET_INSTANCES)

TA_ADD_FLAGS ?= $(TRUSTLET_ADD_FLAGS)

TA_MOBICONFIG_USE ?= $(TRUSTLET_MOBICONFIG_USE)
TA_MOBICONFIG_KEY ?= $(TRUSTLET_MOBICONFIG_KEY)
TA_MOBICONFIG_KID ?= $(TRUSTLET_MOBICONFIG_KID)

TA_OPTS ?= $(TRUSTLET_OPTS)

TASDK_DIR ?= $(TLSDK_DIR)
TASDK_DIR_SRC ?= $(TLSDK_DIR_SRC)

GPENTRY_LIB :=
GP_LEVEL :=
GP_UUIDKEYFILE :=

ifeq ($(GP_ENTRYPOINTS),Y)
    GPENTRY_LIB := $(TASDK_DIR_SRC)/Bin/GpApi/GpApi.lib
    GP_LEVEL := -gp_level GP
    ifeq ($(TA_SERVICE_TYPE),2)
        ifdef TA_UUIDKEYFILE
            GP_UUIDKEYFILE :=  -uuidkeyfile $(TA_UUIDKEYFILE)
        endif
        GP_UUID  := $(shell java -jar $(TASDK_DIR_SRC)/Bin/MobiConvert/MobiConvert.jar $(GP_UUIDKEYFILE) --printuuid | tr -d '-')
    else
        GP_UUID  := $(TA_UUID)
    endif

endif


# Toolchain set up
ifeq ($(TOOLCHAIN),GNU)
   CROSS=arm-none-eabi
   LINKER_SCRIPT=$(TASDK_DIR_SRC)/trusted_application.ld

   CC=$(CROSS_GCC_PATH_BIN)/$(CROSS)-gcc
   LINKER=$(CROSS_GCC_PATH_BIN)/$(CROSS)-ld
   READELF=$(CROSS_GCC_PATH_BIN)/$(CROSS)-readelf
else
   CC=$(ARM_RVCT_PATH_BIN)/armcc
   ASM=$(ARM_RVCT_PATH_BIN)/armasm
   LINKER=$(ARM_RVCT_PATH_BIN)/armlink
   READELF=$(ARM_RVCT_PATH_BIN)/fromelf
endif

# OUTPUT_ROOT used to clean build, keep it defined
#TRUSTED_APP_DIR=Locals/Code
OUTPUT_ROOT := $(TEE_TRUSTLET_OUTPUT_PATH)/$(OUTPUT_NAME)
ifeq ($(TOOLCHAIN),GNU)
   OUTPUT_PATH := $(OUTPUT_ROOT)/$(TOOLCHAIN)/$(TEE_MODE)
else
   OUTPUT_PATH := $(OUTPUT_ROOT)/$(TEE_MODE)
endif
OUTPUT_OBJ_PATH=$(OUTPUT_PATH)/obj-local

# ************************************************************************************************
# Trusted application being build
TA_AXF := $(OUTPUT_PATH)/$(OUTPUT_NAME).axf
TA_LST2 := $(OUTPUT_PATH)/$(OUTPUT_NAME).lst2
ifeq ($(GP_ENTRYPOINTS),Y)
TA_BIN := $(OUTPUT_PATH)/$(TA_UUID).tabin
GP_BIN := $(OUTPUT_PATH)/$(GP_UUID).tabin
else
TA_BIN := $(OUTPUT_PATH)/$(TA_UUID).tlbin
endif
UUID_H := $(OUTPUT_ROOT)/../Public/$(OUTPUT_NAME)_uuid.h
TA_MOBICONFIG_USE ?= false

# ************************************************************************************************
# TASDK
TASDK_SRC_C :=
TASDK_SRC_ASM :=
TASDK_DIR_INC := \
    $(TASDK_DIR_SRC)/Public \
    $(TASDK_DIR_SRC)/Public/MobiCore/inc \
    $(TASDK_DIR_SRC)/Public/GPD_TEE_Internal_API
TLAPI_LIB := $(TASDK_DIR_SRC)/Bin/TlApi/TlApi.lib
TLENTRY_LIB := $(TASDK_DIR_SRC)/Bin/TlEntry/TlEntry.lib




# ************************************************************************************************
# ************************************************************************************************
# ************************************************************************************************
# Architecture specifics
# Should not require changes for Trusted Application

# Generic options
ifeq ($(TOOLCHAIN),GNU)
   ARM_ARCH := ARMv7
   ARM_CHIP := ARMV7_A
   ARM_CPU := generic-armv7-a
   ARM_FPU := vfp
else
   ARM_ARCH := ARMv7
   ARM_CHIP := ARMV7_A
   ARM_CPU := 7-A # for armcc --cpu=xxx
   ARM_FPU := none # for armcc --fpu=xxx
endif
ARM_SHAPE := STD
PLATFORM := $(ARM_CHIP)_$(ARM_SHAPE)

# CC options
ARM_OPT_CC := # init as empty
ARM_OPT_CC += -DPLAT=$(PLATFORM)
ARM_OPT_CC += -DARM_ARCH=$(ARM_ARCH) -D__$(ARM_ARCH)__
ARM_OPT_CC += -D__$(ARM_CHIP)__
ARM_OPT_CC += -D__$(PLATFORM)__
ARM_OPT_CC += -D$(ARM_CHIP)_SHAPE=$(ARM_SHAPE)
ARM_OPT_CC += $(TA_OPTS)
ARM_OPT_CC += -DTBASE_API_LEVEL=$(TBASE_API_LEVEL)

ifeq ($(TOOLCHAIN),GNU)
   ARM_OPT_CC += -mcpu=$(ARM_CPU) -mfpu=$(ARM_FPU) -mfloat-abi=softfp
else
   ARM_OPT_CC += --cpu=$(ARM_CPU) --fpu=$(ARM_FPU)
endif

# ASM options
ARM_OPT_ASM := # init as empty


# ************************************************************************************************
# Debug options

ifeq ($(TEE_MODE),Debug)
   CC_DBG_OPTS := -DDEBUG --debug

   ifeq ($(TOOLCHAIN),GNU)
      LINK_DBG_OPTS :=
   else
      LINK_DBG_OPTS := --debug
   endif

else
   ifeq ($(TOOLCHAIN),GNU)
      LINK_DBG_OPTS := --strip-debug
   else
      LINK_DBG_OPTS := --no_debug --no_comment_section
   endif

endif

# ************************************************************************************************
# ************************************************************************************************
# ************************************************************************************************
# Standard options

ifeq ($(TA_MOBICONFIG_USE),true)
TA_MOBICONFIG_PARAM := --config --in $(TA_AXF) \
            --out $(TA_AXF).conv \
            --key $(TA_MOBICONFIG_KEY) \
            --kid $(TA_MOBICONFIG_KID)
TA_PARAM := -servicetype $(TA_SERVICE_TYPE) \
            -numberofthreads $(TA_NO_OF_THREADS) \
            -numberofinstances $(TA_INSTANCES) \
            -memtype $(TA_MEMTYPE) \
            -flags $(TA_FLAGS) \
            -bin $(TA_AXF).conv \
            -output $(TA_BIN) \
            -keyfile $(TA_KEYFILE) \
            $(GP_LEVEL) \
            $(GP_UUIDKEYFILE) \
            -interfaceversion $(TA_INTERFACE_VERSION) \
            $(TA_ADD_FLAGS)
else
TA_PARAM := -servicetype $(TA_SERVICE_TYPE) \
            -numberofthreads $(TA_NO_OF_THREADS) \
            -numberofinstances $(TA_INSTANCES) \
            -memtype $(TA_MEMTYPE) \
            -flags $(TA_FLAGS) \
            -bin $(TA_AXF) \
            -output $(TA_BIN) \
            -keyfile $(TA_KEYFILE) \
            $(GP_LEVEL) \
            $(GP_UUIDKEYFILE) \
            -interfaceversion $(TA_INTERFACE_VERSION) \
            $(TA_ADD_FLAGS)
endif

ifeq ($(TOOLCHAIN),GNU)
CC_OPTS :=  -DARCH=__arm32__ \
            -D__arm32__ \
            -mthumb \
            -D__THUMB__ \
            -O3 \
            -Os \
            -fno-builtin \
            -mthumb-interwork \
            -fdata-sections \
            -ffunction-sections \
            -fno-short-enums \
            $(ARM_OPT_CC) \
            $(CC_DBG_OPTS)
else
CC_OPTS :=  -D__ARMCC__ \
            -DARCH=__arm32__ \
            -D__arm32__ \
            --thumb \
            -D__THUMB__ \
            --apcs=interwork \
            --reduce_paths \
            --diag_style=gnu \
            --depend_format=unix_escaped \
            --no_depend_system_headers \
            --gnu \
            --bss_threshold=0 \
            --enum_is_int \
            --interface_enums_are_32_bit \
            -O3 \
            -Ospace \
            -J$(ARM_RVCT_PATH_INC) \
            $(ARM_OPT_CC) \
            $(CC_DBG_OPTS) \
            ${ARMCC_COMPILATION_FLAGS}
endif
CC_OPTS += -DTRUSTLET
CC_OPTS += -DTRUSTEDAPP

ifeq ($(TOOLCHAIN),GNU)
LINK_OPTS := --verbose \
            -nostdlib \
            --gc-sections \
            -T $(LINKER_SCRIPT) \
            -o $(TA_AXF) \
            $(LINK_DBG_OPTS)
else
LINK_OPTS := --reduce_paths \
            --diag_style=gnu \
            --datacompressor=off \
            --verbose \
            --map \
            --callgraph \
            --remove \
            --symbols \
            --list=$(OUTPUT_PATH)/$(OUTPUT_NAME).lst \
            --libpath=$(ARM_RVCT_PATH_LIB) \
            --scatter=$(TASDK_DIR_SRC)/trusted_application.sct \
            -o $(TA_AXF) \
            $(LINK_DBG_OPTS)
endif

# ************************************************************************************************
# Actual sets of files to work on
SRC := $(SRC_C) $(SRC_CPP) $(SRC_ASM)
TASDK_SRC := $(TASDK_SRC_C) $(TASDK_SRC_ASM)

SRC_OBJ = $(addprefix $(OUTPUT_OBJ_PATH)/,$(join $(dir $(SRC)), $(addsuffix .o,$(notdir $(basename $(SRC))))))
TASDK_OBJ = $(addprefix $(OUTPUT_OBJ_PATH)/,$(addsuffix .o,$(notdir $(basename $(TASDK_SRC)))))
OBJ := $(SRC_OBJ) $(TASDK_OBJ)

ifeq ($(TOOLCHAIN),GNU)
   INC := $(INCLUDE_DIRS:%=-I%) $(TASDK_DIR_INC:%=-I%) -I$(CROSS_GCC_PATH_INC)
else
   INC := $(INCLUDE_DIRS:%=-I%) $(TASDK_DIR_INC:%=-I%)
endif

ifeq ($(GP_ENTRYPOINTS),Y)
LIBS := $(TLENTRY_LIB) $(GPENTRY_LIB) ${CUSTOMER_DRIVER_LIBS} $(CROSS_GCC_PATH_LIB_LOCAL)
else
LIBS := $(TLENTRY_LIB) $(TLAPI_LIB) ${CUSTOMER_DRIVER_LIBS} $(CROSS_GCC_PATH_LIB_LOCAL)
endif

ifeq ($(TOOLCHAIN),GNU)
   READ_OPT=-a $(TA_AXF) > $(TA_LST2)
   C99=-std=c99
else
   READ_OPT=--dump_build_attributes --decode_build_attributes --text -c -d -e -t -z -a -v -g -r -s --datasymbols $(TA_AXF) --output $(TA_LST2)
   C99=--c99
endif

# ************************************************************************************************
# Rules

ifeq ($(GP_ENTRYPOINTS),Y)
all : $(UUID_H) $(GP_BIN)
else
all : $(TA_BIN)
endif

$(TA_AXF) : $(OBJ) $(LIBS)
	$(info ******************************************)
	$(info ** LINKER ********************************)
	$(info ******************************************)
	$(LINKER) $(LINK_OPTS) $(OBJ) $(LIBS)

$(TA_BIN) : $(TA_AXF)
ifeq ($(TA_MOBICONFIG_USE),true)
	$(info ******************************************)
	$(info ** MobiConfig ****************************)
	$(info ******************************************)
	$(JDK_PATH)/java -jar $(COMP_PATH_MobiConfig)/Bin/MobiConfig.jar $(TA_MOBICONFIG_PARAM)
endif
	$(info ******************************************)
	$(info ** READELF & MobiConvert *****************)
	$(info ******************************************)
	$(READELF) $(READ_OPT)
	$(JDK_PATH)/java -jar $(TASDK_DIR_SRC)/Bin/MobiConvert/MobiConvert.jar $(TA_PARAM) >$(TA_BIN).log

ifeq ($(TA_SERVICE_TYPE),2)
$(GP_BIN) : $(TA_BIN)
	$(info ******************************************)
	$(info ** Copy tabin with GP UUID name **********)
	$(info ******************************************)
	cp $(TA_BIN) $(GP_BIN)
	rm -f $(TA_BIN)
endif

$(UUID_H) :
	$(info ******************************************)
	$(info ** UUID header file creation    **********)
	$(info ******************************************)
	mkdir -p $(OUTPUT_PATH)/../Public
	$(TASDK_DIR_SRC)/uuid2header.sh $(OUTPUT_NAME) $(GP_UUID) $(UUID_H)

.PHONY: clean

clean :
	rm -rf $(OUTPUT_PATH)/*

clean_all :
	rm -rf $(OUTPUT_ROOT)/*/*

$(OUTPUT_OBJ_PATH)/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(C99) $(CC_OPTS) $(CC_OPTS2) $(INC) -c -o $@ $<

$(OUTPUT_OBJ_PATH)/%.o : %.cpp
	mkdir -p $(dir $@)
	$(CC) -cpp $(CC_OPTS) $(INC) -c -o $@ $<

# ************************************************************************************************

