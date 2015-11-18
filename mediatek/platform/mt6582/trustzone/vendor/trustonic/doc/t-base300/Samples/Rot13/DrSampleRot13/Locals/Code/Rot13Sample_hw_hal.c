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

#include "drStd.h"
#include "Rot13Sample_hw_hal.h"

void HAL_Rot13encrypt(uint32_t len, uint8_t *cleartext)
{
	// ROT13 encryption
    	uint32_t i;
	    for (i = 0; i < len; i++) {
			// get char from message buffer
			char c = cleartext[i];
			// convert lower case letter to upper case 
			char upper = (char) (c & (char) 32);
			c &= ~upper;
			// do ROT13 on A-Z, leave any other bytes untouched
			if ((c >= 'A') && (c <= 'Z')) {
			    
                // this implementation uses a division. However, ARM CPUs do 
                // not support this natively, to the library has to provide 
                // code for this. For the RVDS 4.1 library, the additional 
                // div code increases the Trustlet's code segment size by 
                // about 396 byte.
                //    c = ((c - 'A' + 13) % 26 + 'A');
			    
			    // this implementation avoids a division
				c += 13;
				if (c > 'Z') { c -= 26; }
			}
			// restore lower case letter
			c |= upper; 
			// write ROT13 encrypted char back to message buffer
			cleartext[i] = c;
		}
}

