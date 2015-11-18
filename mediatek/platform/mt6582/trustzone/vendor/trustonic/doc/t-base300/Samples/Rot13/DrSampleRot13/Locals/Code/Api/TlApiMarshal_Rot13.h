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
 
#ifndef __TLAPIMARSHAL_ROT13_H__
#define __TLAPIMARSHAL_ROT13_H__

#include "TlApi/TlApiError.h"
#include "TlApiRot13.h"
 
void InitialiseROT13Driver(void);

/** Each function must have its own ID.
 * Extend this list if you add a new function.
 */
typedef enum {
	FID_DRV_ENCODE_ROT13 = 1            /**< Function to do the encryption. */
} rot13FuncID_t;



/** Maximum number of parameters. */
#define MAX_MAR_LIST_LENGTH 8                      /**< Maximum list of possible marshaling parameters. */

/** Union of marshaling parameters. */
/* If adding any function, add the marshaling structure here */
typedef struct {
	uint32_t     functionId;                       /**< Function identifier. */
	struct {
		tlApiEncodeRot13_ptr encodeRot13;
		tlApiResult_t retVal;
		uint32_t parameter[MAX_MAR_LIST_LENGTH];   /* untyped parameter list (expends union to 8 entries) */
	} payload;
} Rot13MarshalingParam_t, *Rot13MarshalingParam_ptr;

/* Driver ID. These is managed by Trustonic */
#define DRV_ROT13SAMPLE_ID DRIVER_ID

_NORETURN static void ROT13DriverIPC_Handler(void);

#endif // __TLAPIMARSHAL_ROT13_H__
