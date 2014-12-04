ifeq ($(strip $(MTK_DRM_KEY_MNG_SUPPORT)),yes)

ifneq ($(wildcard $(MTK_ROOT_CUSTOM_OUT)/drm/FORCE_KB_EKKB),)
PRODUCT_COPY_FILES += $(MTK_ROOT_CUSTOM_OUT)/drm/FORCE_KB_EKKB:data/key_provisioning/FORCE_KB_EKKB 
endif

ifneq ($(wildcard $(MTK_ROOT_CUSTOM_OUT)/drm/KB_EKKB),)
PRODUCT_COPY_FILES += $(MTK_ROOT_CUSTOM_OUT)/drm/KB_EKKB:data/key_provisioning/KB_EKKB
endif

ifneq ($(wildcard $(MTK_ROOT_CUSTOM_OUT)/drm/KB_PM),)
PRODUCT_COPY_FILES += $(MTK_ROOT_CUSTOM_OUT)/drm/KB_PM:data/key_provisioning/KB_PM
endif

endif

