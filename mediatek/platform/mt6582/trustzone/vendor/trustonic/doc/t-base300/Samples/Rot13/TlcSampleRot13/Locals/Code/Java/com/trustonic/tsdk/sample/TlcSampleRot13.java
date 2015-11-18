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

public class TlcSampleRot13 {

    public static native String getDefaultPlainText();
    public static native String rot13NWd(String plainText);

    public static native int open(int spid, byte[] pTAData, int nTASize);

    public static native String rot13SWd(String plainText);
    public static native String unwrapDataSWd(byte[] pData, int nDataSize);

    public static native void close();

    /**
     * This is used to load the library on application startup. The
     * library has already been unpacked to the app specific folder
     * at installation time by the package manager.
     */
    static {
        System.loadLibrary("TlcSampleRot13");
    }
}
