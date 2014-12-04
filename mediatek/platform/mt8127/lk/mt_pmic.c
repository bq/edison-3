#ifdef MTK_PMIC_MT6397
#ifdef MTK_ALPS_BOX_SUPPORT
  #include "mt_pmic_mt6397_dummy.c"
#else
#include "mt_pmic_mt6397.c"
#endif
#else
#include "mt_pmic_mt6323.c"
#endif