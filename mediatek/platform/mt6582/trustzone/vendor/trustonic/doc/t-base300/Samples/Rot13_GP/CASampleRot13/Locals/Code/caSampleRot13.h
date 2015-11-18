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

/*
 * caSampleRot13.h
 *
 */

#ifndef CASAMPLEROT13_H_
#define CASAMPLEROT13_H_

bool caOpen(void);

char *caRot13(const char *plainText);

void caClose(void);

#endif // CASAMPLEROT13_H_
