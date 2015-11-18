include $(MTK_PATH_BUILD)/common.mk
$(call all-modules-src-or-makefile,$(obj),CUSTOM_KERNEL_)
obj-n := dummy.o

ifneq (,$(filter gslx680,$(CUSTOM_KERNEL_TOUCHPANEL)))
obj-y+=gsl_point_id.o
$(obj)/gsl_point_id.o: $(srctree)/$(obj)/touchpanel/gsl_point_id
	cp $(srctree)/$(obj)/touchpanel/gsl_point_id $(obj)/gsl_point_id.o
endif
