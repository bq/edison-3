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

#include "tlStd.h"
#include "TlApi/TlApiCommon.h"
#include "TlApiMarshal_Rot13.h"
#include "TlApiDriver.h"



_TLAPI_EXTERN_C tlApiResult_t tlApiEncodeRot13(tlApiEncodeRot13_ptr encodeData)
{
	Rot13MarshalingParam_t marParam = {
			.functionId = FID_DRV_ENCODE_ROT13,
			.payload    = {
					.encodeRot13 = encodeData
 			}
	};
	tlApiResult_t ret = tlApi_callDriver(DRV_ROT13SAMPLE_ID, &marParam);
	return ret;
}
