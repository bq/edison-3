#include <platform/mtk_boot_share_page.h>

U32 mtk_boot_share_page_get_dev_info_adr(void)
{
    return (U32)BOOT_SHARE_BASE + (U32)BOOT_SHARE_DEV_INFO_OFST;
}
