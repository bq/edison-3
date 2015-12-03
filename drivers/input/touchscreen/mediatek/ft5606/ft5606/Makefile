include $(srctree)/drivers/misc/mediatek/Makefile.custom

# Linux driver folder
ccflags-y += -I$(srctree)/arch/arm/mach-$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)/touchpanel/ft5606/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/ft5606/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/

obj-y	+=  focaltech_ctl.o
obj-y	+=  ft6x06_ex_fun.o
obj-y	+=  ft5606_driver.o

