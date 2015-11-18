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

package com.trustonic.tsdk.sample;

public class TlcSampleSha256 {

	public static native int open(byte[] tabuf, int tabuflen);

	public static native int sha256(byte[] srcbuf, int srcbuflen, byte[] dstbuf, int dstbuflen);

	public static native void close();

	/**
	 * this is used to load the library on application startup. The library has
	 * already been unpacked to the app specific folder at installation time by
	 * the package manager.
	 */
	static {
		System.loadLibrary("TlcSampleSha256");
	}
}

