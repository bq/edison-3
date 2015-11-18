 /*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 * 
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include <stdlib.h>

#include "tlcfoo.h"

#define LOG_TAG "TLC FOO"
#include "log.h"

int main(int argc, char *argv[])
{
    int     c;
    unsigned int option  = 1;	/* 1: add, 2: minus */
    unsigned int num1	 = 6582;
    unsigned int num2	 = 6592;	
    unsigned int result	 = 0;
    mcResult_t ret;

    LOG_I("Copyright ?Trustonic Limited 2013");
	LOG_I("");
	LOG_I("FOO TLC called");

    while ((c = getopt (argc, argv, "am")) != -1) 
	{
        switch (c)
        {
        case 'a':
			/* Add */
			option = 1;
            break;
        case 'm':
			/* Minus */
            option = 2;
            break;
        default:
            LOG_E("Unknown option");
            return 1;
        }
    }

    ret = tlcOpen();
	
    if (MC_DRV_OK != ret) 
    {
        LOG_E("open TL session failed!");
        return ret;
    }

    if(1 == option) 
    {
        // FOO add operation
        ret = add(&num1, &num2, &result);
		LOG_I("Final Result showed in NWd (%d)", result);
    }

    if(2 == option) 
    {
        // FOO minus operation
        ret = minus(&num1, &num2, &result);
        LOG_I("Final Result showed in NWd (%d)", result);
    }	
    
    LOG_I("Final result is %d", result);

exit:    
    tlcClose();
    return ret;
}
