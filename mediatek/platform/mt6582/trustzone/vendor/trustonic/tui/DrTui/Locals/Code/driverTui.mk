################################################################################
#
# <t-base TUI Driver
#
################################################################################

#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------

DRIVER_UUID := 070c0000000000000000000000000000
DRIVER_MEMTYPE := 2 # 0: iram preferred; 1: iram; 2: eram
DRIVER_NO_OF_THREADS := 4 # min =1; max =8
DRIVER_SERVICE_TYPE := 1 # 1: driver; 2: service provider trustlet; 3: system trustlet
DRIVER_FLAGS:= 0 # 0: no flag; 1: permanent; 2: service has no WSM control interface; 3: both (permanent and service has not WSM control interface)
DRIVER_VENDOR_ID :=0 # Trustonic
DRIVER_NUMBER := 0x0101 # DrTui
DRIVER_ID := $(shell echo $$(($(DRIVER_VENDOR_ID)<<16|$(DRIVER_NUMBER))))
DRIVER_INTERFACE_VERSION := 1.0
