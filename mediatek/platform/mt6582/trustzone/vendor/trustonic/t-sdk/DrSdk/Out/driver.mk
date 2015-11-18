# ************************************************************************************************
# Environment
$(info ******************************************)
$(info Driver Build)
$(info ******************************************)

# TEE_MODE : Debug, Release
ifeq ($(PLATFORM),)
$(info - PLATFORM  is  not  set,  default  is : ARM_VE_A9X4_STD)
PLATFORM ?= mt6582_evb
endif

# TEE_MODE : Debug, Release
ifeq ($(TEE_MODE),)
$(info - MODE  is  not  set,  default  is : Debug)
TEE_MODE ?= Debug
endif

# TOOLCHAIN : ARM, GNU
ifeq ($(TOOLCHAIN),)
$(info - TOOLCHAIN is not set, default is : GNU)
TOOLCHAIN ?= GNU
endif

# TBASE_API_LEVEL
ifeq ($(TBASE_API_LEVEL),)
$(info - TBASE_API_LEVEL  is  not  set,  default  is : 2)
TBASE_API_LEVEL ?= 2
endif

$(info ******************************************)
$(info PLATFORM : $(MTK_PROJECT))
$(info TOOLCHAIN : $(TOOLCHAIN))
$(info MODE      : $(TEE_MODE))
$(info TBASE_API_LEVEL : $(TBASE_API_LEVEL))
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


ifeq ($(TOOLCHAIN),GNU)
   CROSS=arm-none-eabi
   LINKER_SCRIPT=$(DRSDK_DIR_SRC)/driver.ld

   CC=$(CROSS_GCC_PATH_BIN)/$(CROSS)-gcc
   LINKER=$(CROSS_GCC_PATH_BIN)/$(CROSS)-ld
   READELF=$(CROSS_GCC_PATH_BIN)/$(CROSS)-readelf
   #ASM=$(CROSS_GCC_PATH_BIN)/$(CROSS)-as
   #We use gcc as preprocessor for GNU asm 	
   ASM=$(CROSS_GCC_PATH_BIN)/$(CROSS)-gcc   
   ARCH=$(CROSS_GCC_PATH_BIN)/$(CROSS)-ar
else
   CC=$(ARM_RVCT_PATH_BIN)/armcc
   ASM=$(ARM_RVCT_PATH_BIN)/armasm
   LINKER=$(ARM_RVCT_PATH_BIN)/armlink
   READELF=$(ARM_RVCT_PATH_BIN)/fromelf
   ARCH=$(ARM_RVCT_PATH_BIN)/armar
endif

# OUTPUT_ROOT used to clean build, keep it defined
OUTPUT_ROOT := $(TEE_DRIVER_OUTPUT_PATH)/$(OUTPUT_NAME)
ifeq ($(TOOLCHAIN),GNU)
   OUTPUT_PATH := $(OUTPUT_ROOT)/$(MTK_PROJECT)/$(TOOLCHAIN)/$(TEE_MODE)
   OUTPUT_PATH_INDEPENDENT := $(OUTPUT_PATH)
else
   OUTPUT_PATH := $(OUTPUT_ROOT)/$(MTK_PROJECT)/$(TEE_MODE)
   OUTPUT_PATH_INDEPENDENT := $(OUTPUT_PATH)
endif
OUTPUT_OBJ_PATH=$(OUTPUT_PATH)/obj-local

# ************************************************************************************************
# Driver being built
DR_AXF := $(OUTPUT_PATH)/$(OUTPUT_NAME).axf
DR_BIN := $(OUTPUT_PATH)/$(DRIVER_UUID).drbin
DR_LST2 := $(OUTPUT_PATH)/$(OUTPUT_NAME).lst2
DR_LIB := $(OUTPUT_PATH_INDEPENDENT)/$(OUTPUT_NAME).lib

# ************************************************************************************************
# DRSDK
DRSDK_SRC_C := 
DRSDK_SRC_ASM := 
DRSDK_DIR_INC := $(DRSDK_DIR_SRC)/Public $(DRSDK_DIR_SRC)/Public/MobiCore/inc $(TLSDK_DIR_SRC)/Public  $(TLSDK_DIR_SRC)/Public/MobiCore/inc
DRAPI_LIB := $(DRSDK_DIR_SRC)/Bin/DrApi/DrApi.lib 
DRENTRY_LIB := $(DRSDK_DIR_SRC)/Bin/DrEntry/DrEntry.lib


# ************************************************************************************************
# ************************************************************************************************
# ************************************************************************************************
# Get architecture specifics files from platform makefile.
#  These values end up in compiler settings and provide handy defines in your source files.
include $(DRSDK_DIR_SRC)/Public/Platforms/*.mk

ARM_PLAT := $(ARM_CHIP)_$(ARM_SHAPE)

# ************************************************************************************************
# CC options
ARM_OPT_CC := # init as empty 
ARM_OPT_CC += -DPLAT=$(ARM_PLAT)
ARM_OPT_CC += -DARM_ARCH=$(ARM_ARCH) -D__$(ARM_ARCH)__
ARM_OPT_CC += -D__$(ARM_CHIP)__
ARM_OPT_CC += -D__$(ARM_PLAT)__
ARM_OPT_CC += -D$(ARM_CHIP)_SHAPE=$(ARM_SHAPE) 
ARM_OPT_CC += -DTBASE_API_LEVEL=$(TBASE_API_LEVEL)

ifeq ($(TOOLCHAIN),GNU)
   ARM_OPT_CC += -mcpu=$(ARM_CPU) -mfpu=$(ARM_FPU) -mfloat-abi=softfp
else
   ARM_OPT_CC += --cpu=$(ARM_CPU) --fpu=$(ARM_FPU)
endif

ARM_OPT_ASM := # init as empty 
ifeq ($(TOOLCHAIN),GNU)
	ARM_OPT_ASM += -DPLAT=$(ARM_PLAT)
    ARM_OPT_ASM += -DARM_ARCH=$(ARM_ARCH) -D__$(ARM_ARCH)__
    ARM_OPT_ASM += -D__$(ARM_CHIP)__
    ARM_OPT_ASM += -D__$(ARM_PLAT)__
    ARM_OPT_ASM += -D$(ARM_CHIP)_SHAPE=$(ARM_SHAPE) 

	ARM_OPT_ASM += -mcpu=$(ARM_CPU) -mfpu=$(ARM_FPU) -mfloat-abi=softfp

else	
	ARM_OPT_ASM += 
    ARM_OPT_ASM += --cpreproc_opts='-DPLAT=$(ARM_PLAT) \
                                    -DARM_ARCH=$(ARM_ARCH) \
                                    -D__$(ARM_ARCH)__ \
                                    -D__$(ARM_CHIP)__ \
                                    -D__$(ARM_PLAT)__ \
                                    -D$(ARM_CHIP)_SHAPE=$(ARM_SHAPE)'

	ARM_OPT_ASM += --cpu=$(ARM_CPU)
endif

# ************************************************************************************************
# Debug options

ifeq ($(TEE_MODE),Debug)
   CC_DBG_OPTS := -DDEBUG --debug

ifeq ($(TOOLCHAIN),GNU)
       ASM_DBG_OPTS :=  -Wa,-g,--keep-locals
else
   ASM_DBG_OPTS :=  -g \
                    --keep
endif

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

DR_PARAM := -servicetype $(DRIVER_SERVICE_TYPE) \
            -numberofthreads $(DRIVER_NO_OF_THREADS) \
            -bin $(DR_AXF) \
            -output $(DR_BIN) \
            -d $(DRIVER_ID) \
            -memtype $(DRIVER_MEMTYPE) \
            -flags $(DRIVER_FLAGS) \
            -interfaceversion $(DRIVER_INTERFACE_VERSION) \
            -keyfile $(DRIVER_KEYFILE)

ifeq ($(TOOLCHAIN),GNU)
CC_OPTS_LIB :=  -DARCH=__arm32__ \
            -D__arm32__ \
            -mthumb \
            -D__THUMB__ \
            -D__time_t_defined \
            -O3 \
            -Os \
            -fno-builtin \
            -mthumb-interwork \
            -fdata-sections \
            -ffunction-sections \
            -fno-short-enums \
            $(ARM_OPT_CC) \
            $(ARMCC_COMPILATION_FLAGS)

else
CC_OPTS_LIB := -D__ARMCC__ \
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
            $(ARMCC_COMPILATION_FLAGS)

endif

CC_OPTS :=  $(CC_OPTS_LIB) \
            $(CC_DBG_OPTS) 

CC_OPTS += -DDRIVER

ifeq ($(TOOLCHAIN),GNU)
	ASM_OPTS := -mthumb-interwork \
                -DARCH=__arm32__ \
                -D__arm32__ \
                -D__THUMB__ \
                $(ARM_OPT_ASM) \
                ${ARMCC_COMPILATION_FLAGS} \
                $(ASM_DBG_OPTS) \
                -c

else
    ASM_OPTS := --arm \
                --apcs=interwork \
                --reduce_paths \
                --diag_style=gnu \
                --cpreproc \
                --cpreproc_opts='-D__ARMCC__ -DARCH=__arm32__ -D__arm32__ -D__THUMB__' \
                $(ARM_OPT_ASM) \
                --cpreproc_opts='${ARMCC_COMPILATION_FLAGS}' \
                $(ASM_DBG_OPTS)
endif

ifeq ($(TOOLCHAIN),GNU)
LINK_OPTS := --verbose \
            -nostdlib \
            --gc-sections \
            -T $(LINKER_SCRIPT) \
            -o $(DR_AXF) \
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
             --scatter=$(DRSDK_DIR_SRC)/dr.sct \
             -o $(DR_AXF) \
             $(LINK_DBG_OPTS) \
             $(EXTRA_LINK_OPTS)
endif

ifeq ($(TOOLCHAIN),GNU)
ARCHIVER_OPTS := -rcs
else
ARCHIVER_OPTS := --create \
             --debug_symbols \
             -v
endif
			
# ************************************************************************************************
# Actual sets of files to work on
SRC := $(SRC_C) $(SRC_CPP) $(SRC_ASM)
DRSDK_SRC := $(DRSDK_SRC_C) $(DRSDK_SRC_ASM)

SRC_OBJ = $(addprefix $(OUTPUT_OBJ_PATH)/,$(join $(dir $(SRC)), $(addsuffix .o,$(notdir $(basename $(SRC))))))
DRSDK_OBJ = $(addprefix $(OUTPUT_OBJ_PATH)/,$(addsuffix .o,$(notdir $(basename $(DRSDK_SRC))))) 
OBJ := $(SRC_OBJ) $(DRSDK_OBJ)

ifneq ($(strip $(SRC_LIB_C)),)
DR_LIB_OBJ = $(addprefix $(OUTPUT_OBJ_PATH)/,$(addsuffix .ol,$(notdir $(basename $(SRC_LIB_C)))))
endif


ifeq ($(TOOLCHAIN),GNU)
   INC := $(INCLUDE_DIRS:%=-I%) $(DRSDK_DIR_INC:%=-I%) -I$(CROSS_GCC_PATH_INC)
else
   INC := $(INCLUDE_DIRS:%=-I%) $(DRSDK_DIR_INC:%=-I%)
endif

LIBS := $(DRAPI_LIB) $(DRENTRY_LIB) $(EXTRA_LIBS) $(CROSS_GCC_PATH_LIB_LOCAL)

ifeq ($(TOOLCHAIN),GNU)
   READ_OPT=-a $(DR_AXF) > $(DR_LST2)
   C99=-std=c99
else
   READ_OPT=--dump_build_attributes --decode_build_attributes --text -c -d -e -t -z -a -v -g -r -s --datasymbols $(DR_AXF) --output $(DR_LST2)
   C99=--c99
endif

# ************************************************************************************************
# Rules

ifdef DR_LIB_OBJ
all : $(DR_BIN) $(DR_LIB)
else
all : $(DR_BIN)
endif

$(DR_AXF) : $(OBJ) $(LIBS)
	$(info ******************************************)
	$(info ** LINKER ********************************)
	$(info ******************************************)
	$(LINKER) $(LINK_OPTS) $(OBJ) $(LIBS)

$(DR_BIN) : $(DR_AXF)
ifeq ($(TRUSTLET_MOBICONFIG_USE),true)
	$(info ******************************************)
	$(info ** MobiConfig ****************************)
	$(info ******************************************)
	$(JDK_PATH)/java -jar $(COMP_PATH_MobiConfig)/Bin/MobiConfig.jar $(DR_MOBICONFIG_PARAM)
endif
	$(info ******************************************)
	$(info ** READELF & MobiConvert *****************)
	$(info ******************************************)
	$(READELF) $(READ_OPT)
	$(JDK_PATH)/java -jar $(DRSDK_DIR_SRC)/Bin/MobiConvert/MobiConvert.jar $(DR_PARAM) >$(DR_BIN).log
	cp $(DR_BIN) $(OUTPUT_PATH)/$(DRIVER_UUID).tlbin


$(DR_LIB) : $(DR_LIB_OBJ)
	$(ARCH) $(ARCHIVER_OPTS) $(DR_LIB) $(DR_LIB_OBJ) 
	
.PHONY: clean

clean:
	rm -rf $(OUTPUT_PATH)/*

clean_all:
	rm -rf $(OUTPUT_ROOT)/*/*/*

$(OUTPUT_OBJ_PATH)/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(C99) $(CC_OPTS) $(INC) -c -o $@ $<

$(OUTPUT_OBJ_PATH)/%.o : %.cpp
	mkdir -p $(dir $@)
	$(CC) --cpp $(CC_OPTS) $(INC) -c -o $@ $<

$(OUTPUT_OBJ_PATH)/%.ol : $(DRLIB_DIR)/%.c
	mkdir -p $(OUTPUT_PATH_INDEPENDENT)
	$(CC) $(C99) $(CC_OPTS_LIB) $(INC) -c -o $@ $<

$(OUTPUT_OBJ_PATH)/%.o : %.s
	mkdir -p $(dir $@)
	$(ASM) $(ASM_OPTS) $(INC) -o $@ $<

$(OUTPUT_OBJ_PATH)/%.o : %.S
	mkdir -p $(dir $@)
	$(ASM) $(ASM_OPTS) $(INC) -o $@ $<
# ************************************************************************************************

