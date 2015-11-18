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

import java.io.InputStream;
import java.io.IOException;
import java.io.BufferedInputStream;
import java.lang.Integer;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.opengl.GLSurfaceView;
import android.content.Context;
import android.content.Intent;
import javax.microedition.khronos.egl.EGLConfig;;
import javax.microedition.khronos.opengles.GL10;
import android.opengl.GLSurfaceView.Renderer;
import android.widget.TextView;
import android.widget.EditText;
import android.content.res.AssetFileDescriptor;
import android.graphics.Color;

 import com.trustonic.tsdk.sample.TlcSampleAes;

public class TSdkSample extends Activity 
{
    private static final String TAG = TSdkSample.class.getSimpleName();
    
    private Activity activity=this;
    private Button ButtonProvisioning = null;
    private Button ButtonAes = null;
    private Button ButtonRot13 = null;
    private Button ButtonRsa = null;
    private Button ButtonSha256 = null;
    private byte[] SampleTaAes = null;
    private byte[] SampleTaRot13 = null;
    private byte[] SampleTaRsa = null;
    private byte[] SampleTaSha256 = null;
    private byte[] mUuid = null;
    private Provision mProvision1 = null;
    private Provision mProvision2 = null;
    private Provision mProvision3 = null;
    private Provision mProvision4 = null;
    private GLSurfaceView mGLView = null;    
    private GLSurfaceView.Renderer renderer = null;
    private String provisioningStatus = "INIT";
    private boolean fIsProvisioningDone = false;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        Button b;
        TextView t;
        Log.d(Constants.LOG_TAG, "onCreate.");
        
        super.onCreate(savedInstanceState);
        mGLView = new GLSurfaceView(this);
        mGLView.setRenderer(renderer);
        setContentView(mGLView);
        getActionBar().setTitle(R.string.home_actionbar_title);
        getActionBar().setSubtitle(R.string.home_actionbar_subtitle);
        getActionBar().setIcon(R.drawable.t_logo);
        setContentView(R.layout.main);

        // Restore from eventual previous state
        if(savedInstanceState != null)  {
            provisioningStatus = savedInstanceState.getString("PROVISIONING_FLAG");
            if(provisioningStatus == "DONE")
            {
                Log.d(TAG, "Provisioning already done.");
                fIsProvisioningDone = true;
                t=(TextView)activity.findViewById(R.id.provisioningText); 
                t.setText(R.string.provisioning_ok_text);
                t.setTextColor(Color.GREEN);
            }
            else if(provisioningStatus == "FAILED") {
                fIsProvisioningDone = false;
                t=(TextView)activity.findViewById(R.id.provisioningText); 
                t.setText(R.string.provisioning_ko_text);
                t.setTextColor(Color.RED);                
            }
        }

        setupUI();

        if(provisioningStatus == "INIT") 
        {
            Log.d(TAG, "Provisioning Samples now...");
            provisionSamples();
        }

        try
        {
            InputStream is;

            is = getResources().getAssets().open(taName(activity.getResources().getString(R.string.AesUuid)));
            int size = is.available();
            SampleTaAes = new byte[size];
            is.read(SampleTaAes);
            is.close();
        }
        catch (IOException e)
        {
            Log.e(TAG, "Opening AES Trusted App failed: Exception", e);
            finish();
        }

        try
        {
            InputStream is;
            is = getResources().getAssets().open(taName(activity.getResources().getString(R.string.Rot13Uuid)));
            int size = is.available();
            SampleTaRot13 = new byte[size];
            is.read(SampleTaRot13);
            is.close();
        }
        catch (IOException e)
        {
            Log.e(TAG, "Opening Rot13 Trusted App failed");
            finish();
        }

        try
        {
            InputStream is;
            is = getResources().getAssets().open(taName(activity.getResources().getString(R.string.RsaUuid)));
            int size = is.available();
            SampleTaRsa = new byte[size];
            is.read(SampleTaRsa);
            is.close();
        }
        catch (IOException e)
        {
            Log.e(TAG, "Opening RSA Trusted App failed");
            finish();
        }

        try
        {
            InputStream is;
            is = getResources().getAssets().open(taName(activity.getResources().getString(R.string.Sha256Uuid)));
            int size = is.available();
            SampleTaSha256 = new byte[size];
            is.read(SampleTaSha256);
            is.close();
        }
        catch (IOException e)
        {
            Log.e(TAG, "Opening SHA-256 Trusted App failed");
            finish();
        }

        // SPPA 2.0: 
        //public SPContainerStructure getSpCont()
    }
    
    public void provisionSamples()
    {
        // Create Service Provider Provisioning Agent (SPPA) instance.
        mProvision1 = new Provision(activity);
        if(mProvision1 != null) {
            // Install TA.
            mProvision1.install(R.string.Rot13Uuid, R.id.home_button_rot13);
        }
        
        mProvision2 = new Provision(activity);
        if(mProvision2 != null) {
            // Install TA.
            mProvision2.install(R.string.AesUuid, R.id.home_button_aes);
        }

        mProvision3 = new Provision(activity);
        if(mProvision3 != null) {
            // Install TA.
           mProvision3.install(R.string.RsaUuid, R.id.home_button_rsa);
        }

        mProvision4 = new Provision(activity);
        if(mProvision4 != null) {
            // Install TA.
           mProvision4.install(R.string.Sha256Uuid, R.id.home_button_sha256);
        }
    }
    
    @Override
    protected void onPause() {
        Log.d(Constants.LOG_TAG, "onPause.");
        super.onPause();
        mGLView.onPause();
    }

    @Override
    protected void onResume() {
        Log.d(Constants.LOG_TAG, "onResume.");
        super.onResume();
        mGLView.onResume();
    }

    protected void onDestroy() {
        Log.d(Constants.LOG_TAG, "onDestroy.");
        super.onDestroy();
        
        // Exit SPPA.
        Log.d(Constants.LOG_TAG, "Exiting SPPAs.");
        if(mProvision1 != null) {
            mProvision1.exit();
        }
        if(mProvision2 != null) {
            mProvision2.exit();
        }
        if(mProvision3 != null) {
            mProvision3.exit();
        }
        if(mProvision4 != null) {
            mProvision4.exit();
        }
    }

  @Override
  protected void onSaveInstanceState(Bundle outState){
      super.onSaveInstanceState(outState);
      TextView t;
      t=(TextView)activity.findViewById(R.id.provisioningText); 
      String status = t.getText().toString();

      if(status.equals(getString(R.string.provisioning_ok_text)) ) {
            provisioningStatus = "DONE";
            Log.d(Constants.LOG_TAG, "Previous status: Provisioning successful. Nothing to do.");
      } else
     if(status.equals(getString(R.string.provisioning_ko_text)) ) {
         provisioningStatus = "FAILED";
            Log.d(Constants.LOG_TAG, getString(R.string.provisioning_ko_text));
     } else {
         provisioningStatus = "INIT";
         Log.d(Constants.LOG_TAG, "Previous status: Provisioning not done. Provisioning samples now...");
     }
     outState.putString("PROVISIONING_FLAG", provisioningStatus);
  }

    private void setupUI(){ 
        // Disable all buttons
        Button b;
            
        // Set default plain text to cipher.
        String plainText = activity.getResources().getString(R.string.rot13_default_text);
        EditText editText = (EditText)findViewById(R.id.rot13_plainText);
        editText.setText(plainText);
        editText.setEnabled(fIsProvisioningDone);
        TextView textView=(TextView)findViewById(R.id.provisioningText); 
        textView.setEnabled(fIsProvisioningDone);

        b = (Button)findViewById(R.id.home_button_rot13);
        b.setEnabled(fIsProvisioningDone);
        b = (Button)findViewById(R.id.home_button_rsa);
        b.setEnabled(fIsProvisioningDone);
        b = (Button)findViewById(R.id.home_button_aes);
        b.setEnabled(fIsProvisioningDone);
        b = (Button)findViewById(R.id.home_button_sha256);
        b.setEnabled(fIsProvisioningDone);

        if(fIsProvisioningDone)
        {
            TextView t; 
            t=(TextView)activity.findViewById(R.id.provisioningText); 
            t.setText(R.string.provisioning_ok_text);
        }  

        ButtonAes = (Button) findViewById(R.id.home_button_aes);
        ButtonAes.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.i(TAG, "Starting AES sample");
                TSdkSample s = ((TSdkSample)v.getContext());

                if (TlcSampleAes.open(s.SampleTaAes, s.SampleTaAes.length) != 0) {
                    Log.e(TAG, "AES sample failed");
                } else {
                    int ret;
                    byte in[] = new byte[512];
                    byte out[] = new byte[512+512];
                    java.util.Arrays.fill(in, (byte)0xab);

                    ret = TlcSampleAes.aes(in, in.length, out, out.length);
                    if (ret == 0)
                    {
                        Log.i(TAG, "AES encryption/decryption succeeded");
                    }
                    else
                    {
                        Log.e(TAG, "AES encryption/decryption failed");
                    }

                    ret = TlcSampleAes.secureObject(in, in.length, out, out.length);
                    if (ret == 0)
                    {
                        Log.i(TAG, "AES wrapping/unwrapping succeeded");
                    }
                    else
                    {
                        Log.e(TAG, "AES wrapping/unwrapping failed");
                    }

                    Log.i(TAG, "AES sample succeeded");
                    TlcSampleAes.close();
                }
            }
        });

        ButtonRot13 = (Button) findViewById(R.id.home_button_rot13);
        ButtonRot13.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                        Log.i(TAG, "Starting ROT13 sample");
        
                        String cipherText = rot13SWd(v);
        
                        Log.i(TAG, "ROT13 sample succeeded");
                        
                        TextView outText = (TextView)findViewById(R.id.rot13_cipherText);
                        outText.setText(cipherText);                        
            }
        });

        ButtonRsa = (Button) findViewById(R.id.home_button_rsa);
        ButtonRsa.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.i(TAG, "Starting Rsa sample");
                TSdkSample s = ((TSdkSample)v.getContext());

                if (TlcSampleRsa.open(s.SampleTaRsa, s.SampleTaRsa.length) != 0) {
                    Log.e(TAG, "RSA sample failed");
                } else {
                    int ret;
                    byte out[] = new byte[636];
                    int length[] = new int[1];

                    ret = TlcSampleRsa.rsa(out, out.length, length);
                    if (ret == 0)
                    {
                        Log.i(TAG, "RSA command succeeded length="+length[0]);
                    }
                    else
                    {
                        Log.e(TAG, "RSA command failed");
                    }

                    Log.i(TAG, "RSA sample succeeded");
                    TlcSampleRsa.close();
                }
            }
        });

        ButtonSha256 = (Button) findViewById(R.id.home_button_sha256);
        ButtonSha256.setOnClickListener(new OnClickListener() {

            private final char[] hexCode = "0123456789ABCDEF".toCharArray();

            public String printHexBinary(byte[] data) {
                StringBuilder r = new StringBuilder(data.length*2);
                for ( byte b : data) {
                    r.append(hexCode[(b >> 4) & 0xF]);
                    r.append(hexCode[(b & 0xF)]);
                }
                return r.toString();
            }

            @Override
            public void onClick(View v) {
                Log.i(TAG, "Starting SHA256 sample");
                TSdkSample s = ((TSdkSample)v.getContext());

                if (TlcSampleSha256.open(s.SampleTaSha256, s.SampleTaSha256.length) != 0) {
                    Log.e(TAG, "SHA256 sample failed");
                } else {
                    int ret;
                    String tmp = "The quick brown fox jumps over the lazy dog\0";
                    byte[] plainText;
                    byte out[] = new byte[32];

                    try {plainText = tmp.getBytes("US-ASCII");}
                    catch(java.io.UnsupportedEncodingException e) {TlcSampleSha256.close();return;}

                    ret = TlcSampleSha256.sha256(plainText, plainText.length, out, out.length);
                    if (ret == 0)
                    {
                        tmp = printHexBinary(out);
                        Log.i(TAG, "SHA256 hash succeeded");
                        Log.i(TAG, "hash:"+tmp);
                    }
                    else
                    {
                        Log.e(TAG, "SHA256 hash failed");
                    }

                    Log.i(TAG, "SHA256 sample succeeded");
                    TlcSampleSha256.close();
                }
            }
        });
    }

        // SWd Rot13 ciphering.
    public String rot13SWd(View v) {
        Log.d(Constants.LOG_TAG, "rot13SWd.");

        TSdkSample s = ((TSdkSample)v.getContext());
        
        // Read plain text.
        EditText inText = (EditText)findViewById(R.id.rot13_plainText);
        String plainText = inText.getText().toString();
        Log.d(Constants.LOG_TAG, "Plain text [" + plainText + "].");

        // Read TA.
        int nTASize = 0;
        byte pTAData[] = null;
        try {
            AssetFileDescriptor inFd =
                getResources().getAssets().openFd(taName(activity.getResources().getString(R.string.Rot13Uuid)));
            nTASize = (int)inFd.getLength();
            pTAData = new byte[nTASize];
            BufferedInputStream in = new BufferedInputStream(inFd.createInputStream());
            in.read(pTAData, 0, nTASize);
            in.close();
        } catch(Exception e) {
            Log.e(Constants.LOG_TAG, "Error: reading TA failed, " + e);
        }

        // Cipher in SWd.
        String spid = activity.getResources().getString(R.string.spid);
        if (TlcSampleRot13.open(Integer.parseInt(spid), s.SampleTaRot13, s.SampleTaRot13.length) != 0) {
            Log.e(TAG, "ROT13 sample failed");
        }
        String cipherText = TlcSampleRot13.rot13SWd(plainText);
        TlcSampleRot13.close();

        return  cipherText;
    }

        // Get TA filename of SPApp.
    public String taName(String uuid) {
        return uuid + ".tlbin";
    }
}
