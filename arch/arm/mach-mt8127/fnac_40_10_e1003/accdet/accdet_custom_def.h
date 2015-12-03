// use accdet + EINT solution
#define ACCDET_EINT
// support multi_key feature
#define ACCDET_MULTI_KEY_FEATURE

//start-xmyyq-20150812-add ACCDET twice report feature
// support multi_key twice report feature
#define ACCDET_MULTI_KEY_TWICE_REPORT_FEATURE
//end-xmyyq-20150812-add ACCDET twice report feature

// after 5s disable accdet
#define ACCDET_LOW_POWER

//#define ACCDET_PIN_RECOGNIZATION
#define ACCDET_28V_MODE

#define ACCDET_SHORT_PLUGOUT_DEBOUNCE
#define ACCDET_SHORT_PLUGOUT_DEBOUNCE_CN 20

//extern struct headset_mode_settings* get_cust_headset_settings(void);
//extern int get_long_press_time_cust(void);