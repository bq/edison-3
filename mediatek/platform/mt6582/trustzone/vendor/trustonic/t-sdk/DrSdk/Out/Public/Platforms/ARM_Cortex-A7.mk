
#-------------------------------------------------------------------------
#
# ARM Cortex-A7
#
#-------------------------------------------------------------------------


#-------------------------------------------------------------------------
# add our supported targets 
ADD_PLATFORM_TARGETS := \
     CORTEXA9_PB_STD \
     ARM_VE_A9X4_STD \
     ARM_VE_A9X4_QEMU \
     mt6582_evb

#-------------------------------------------------------------------------
# specific platforms
VALID_PLATFORM = $(empty)

ifeq ($(PLATFORM),CORTEXA9_PB_STD)
    VALID_PLATFORM = TRUE
    ARM_SHAPE := PB
endif

ifeq ($(PLATFORM),ARM_VE_A9X4_STD)
    VALID_PLATFORM = TRUE
    ARM_SHAPE := VE
    PLATFORM_CORE_COUNT := 4
    PLATFORM_CLUSTER_COUNT := 1
endif

ifeq ($(PLATFORM),ARM_VE_A9X4_QEMU)
    VALID_PLATFORM = TRUE
    ARM_SHAPE := VE
    ARMCC_COMPILATION_FLAGS += -DDR_ARM_VE_A9X4_QEMU
endif

ifeq ($(PLATFORM),mt6582_evb)
    VALID_PLATFORM = TRUE
    ARM_SHAPE := VE
    PLATFORM_CORE_COUNT := 4
    PLATFORM_CLUSTER_COUNT := 1
endif

#-------------------------------------------------------------------------
# platform generic
ifeq ($(VALID_PLATFORM),TRUE)
	ARM_VENDOR := ARM
	ARM_CHIP := CORTEXA7
	ARM_ARCH := ARMv7

    ifeq ($(TOOLCHAIN),GNU)
       ARM_CPU := cortex-a9
       ARM_FPU := vfp
    else
       ARM_CPU := Cortex-A9.no_neon.no_vfp # for armcc --cpu=xxx
       ARM_FPU := SoftVFP # for armcc --fpu=xxx
    endif

	ARM_SUBDIR := CortexA7
endif

