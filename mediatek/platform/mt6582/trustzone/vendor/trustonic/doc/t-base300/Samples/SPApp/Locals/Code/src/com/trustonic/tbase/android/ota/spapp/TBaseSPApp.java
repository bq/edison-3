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

package com.trustonic.tbase.android.ota.spapp;

import java.io.BufferedInputStream;
import java.io.InputStream;

import android.app.Activity;
import android.content.res.AssetManager;
import android.content.res.AssetFileDescriptor;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.trustonic.tsdk.sample.TlcSampleRot13;

// TBase Service Provider Application (SPApp) example Rot13.
public class TBaseSPApp extends Activity {
    private Provision mProvision = null;

    // SPApp native library and function example.
    static {
        System.loadLibrary("SPApp");
    }
    public native int foo();

    protected void onCreate(Bundle savedInstanceState) {
        Log.d(Constants.LOG_TAG, "onCreate.");
        super.onCreate(savedInstanceState);
        Log.i(Constants.LOG_TAG, BuildTag.BUILD_TAG + ".");

        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.main);

        // Set default plain text to cipher.
        String plainText = TlcSampleRot13.getDefaultPlainText();
        Log.d(Constants.LOG_TAG, "Plain text [" + plainText + "]");
        EditText inText = (EditText)findViewById(R.id.plainText);
        inText.setText(plainText);

        // Install SPApp Trusted Application (TA) at launch.
        if(mProvision == null) {
            // Create Service Provider Provisioning Agent (SPPA) instance.
            Log.d(Constants.LOG_TAG, "Creating SPPA.");
            mProvision = new Provision(this);
            // Install TA.
            Log.d(Constants.LOG_TAG, "Installing TA.");
            mProvision.install();
        }
    }

    protected void onDestroy() {
        Log.d(Constants.LOG_TAG, "onDestroy.");
        super.onDestroy();

        // Exit SPPA.
        if(mProvision != null) {
            Log.d(Constants.LOG_TAG, "Exiting SPPA.");
            mProvision.exit();
        }
    }

    // SWd Rot13 ciphering.
    public void rot13SWd(View v) {
        Log.d(Constants.LOG_TAG, "rot13SWd.");

        // Read plain text.
        EditText inText = (EditText)findViewById(R.id.plainText);
        String plainText = inText.getText().toString();
        Log.d(Constants.LOG_TAG, "Plain text [" + plainText + "].");

        // Read TA.
        int nTASize = 0;
        byte pTAData[] = null;
        try {
            AssetFileDescriptor inFd =
                getResources().getAssets().openFd(mProvision.taName());
            nTASize = (int)inFd.getLength();
            pTAData = new byte[nTASize];
            BufferedInputStream in = new BufferedInputStream(inFd.createInputStream());
            in.read(pTAData, 0, nTASize);
            in.close();
        } catch(Exception e) {
            Log.e(Constants.LOG_TAG, "Error: reading TA failed, " + e);
        }

        // Cipher in SWd.
        TlcSampleRot13.open(mProvision.spid(), pTAData, nTASize);
        String cipherText = TlcSampleRot13.rot13SWd(plainText);
        TlcSampleRot13.close();

        // Show ciphered text.
        Log.d(Constants.LOG_TAG, "SWd cipher text [" + cipherText + "].");
        TextView outText = (TextView)findViewById(R.id.cipherText);
        outText.setText(cipherText);
    }

    // NWd Rot13 ciphering.
    public void rot13NWd(View v) {
        Log.d(Constants.LOG_TAG, "rot13NWd.");

        // Read plain text.
        EditText inText = (EditText)findViewById(R.id.plainText);
        String plainText = inText.getText().toString();
        Log.d(Constants.LOG_TAG, "Plain text [" + plainText + "].");

        // Cipher in NWd.
        String expectedCipherText = TlcSampleRot13.rot13NWd(plainText);
        Log.d(Constants.LOG_TAG,
                "NWd cipher text [" + expectedCipherText + "].");

        // Show ciphered text.
        TextView outText = (TextView)findViewById(R.id.expectedCipherText);
        outText.setText(expectedCipherText);
    }

    // Uninstall containers for purpose of installing again TA on next launch.
    public void uninstallRot13SWd(View v) {
        Log.d(Constants.LOG_TAG, "uninstallRot13SWd.");

        mProvision.uninstall();
    }
}
