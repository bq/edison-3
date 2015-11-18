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
import java.util.concurrent.TimeUnit;
import java.lang.Exception;
import android.app.Activity;
import android.util.Log;
import android.widget.Button;
import android.os.AsyncTask;
import android.widget.EditText;
import android.widget.TextView;
import android.graphics.Color;

import com.trustonic.tbase.ota.sppa.ifc.Utils;
import com.trustonic.tbase.ota.sppa.ifc.SPPA;
import com.trustonic.tbase.ota.sppa.ifc.ISPPAService.ProgressCallback;
import com.trustonic.tbase.ota.sppa.ifc.ISPPAService.ProgressState;

// Class for Service Provider Application (SPApp) to interact with Service
// Provider Provisioning Agent (SPPA).
public class Provision {
    private Activity mActivity = null;
    private SPPA mSPPA = null;
    private Service mService = null;
    private ProvisionCallback mCb = null;
    private boolean mFinishedConnect = false;
    private static final int WAIT_TRY = 20;
    private static final int WAIT_PERIOD = 200;
    private boolean mPendingInstall = false;
    private boolean mInstallError = false;
    private int mPendingInstallButton = 0;
    private String mEnrollmentUrl = null;
    private byte[] mSpKey = null;
    private byte[] mUuid = null;
    private String sUuid = null;
    private byte[] mKey = null;
    private int mButtonId = 0;
 
    // SPPA callback for SPApp.
    public class ProvisionCallback implements ProgressCallback {
        public void progress(ProgressState state) {
            TextView t; 
            
            Log.d(Constants.LOG_TAG,
                    "Provision received from SPPA progress state " +
                    state + ".");

            switch(state) {
                case CONNECTED_ROOTPA:
                    Log.d(Constants.LOG_TAG, "Connected to RootPA service.");
                    mFinishedConnect = true;
                    break;
                case DISCONNECTED_ROOTPA:
                    Log.d(Constants.LOG_TAG,
                            "Disconnected to RootPA service.");
                    mFinishedConnect = false;
                    break;
                case CONNECTING_SE:
                    Log.d(Constants.LOG_TAG, "Connecting to Service Enabler.");
                    break;
                case CREATING_ROOT_CONTAINER:
                    Log.d(Constants.LOG_TAG, "Creating Root container.");
                    break;
                case CREATING_SP_CONTAINER:
                    Log.d(Constants.LOG_TAG, "Creating Sp container.");
                    break;
                case FINISHED_PROVISIONING:
                    Log.d(Constants.LOG_TAG,
                            "Provisioning has finished successfully.");
                    t=(TextView)mActivity.findViewById(R.id.provisioningText); 
                    t.setText(R.string.provisioning_ok_text);
                    t.setTextColor(Color.GREEN);
                    break;
                case FINISHED_SE_INSTALL:
                    Log.d(Constants.LOG_TAG,
                            "Developer TA installation finished successfully.");
                    break;
                case ERROR_ROOTPA:
                    Log.e(Constants.LOG_TAG,
                            "Error: RootPA and/or Service Enabler operation failed.");
                    mFinishedConnect = false;
                    mInstallError = true;
                    break;
                case CONNECTING_SM:
                    Log.d(Constants.LOG_TAG, "Connecting to Service Manager.");
                    break;
                case EXECUTE_SM:
                    Log.d(Constants.LOG_TAG, "Execute Service Manager operation.");
                    break;
                case FINISHED_SM:
                    Log.d(Constants.LOG_TAG,
                            "Service Manager operation finished sucessfully.");
                    mActivity.runOnUiThread(new Runnable() {
                        public void run() {
                            TextView t; 
                            t=(TextView)mActivity.findViewById(R.id.provisioningText); 
                            t.setText(R.string.provisioning_ok_text);
                           Button b;
                            mPendingInstall = false;
                            if(Constants.PROGRESS_INDICATOR_ENABLED) {
                                mActivity.setProgressBarIndeterminateVisibility(false);
                            }
                            Log.d(Constants.LOG_TAG,
                                    "Setting Button State.");
                            b = (Button)mActivity.findViewById(mPendingInstallButton);
                            b.setEnabled(true);
                            if( sUuid.equals(mActivity.getResources().getString(R.string.Rot13Uuid)) ){
                                EditText e = (EditText)mActivity.findViewById(R.id.rot13_plainText);
                                e.setEnabled(true);
                            }
                        }
                    });
                    break;
                case ERROR_SPPA:
                    Log.e(Constants.LOG_TAG,
                            "Error: SPPA and/or Service Manager operation failed.");
                    mPendingInstall = false;
                    mActivity.runOnUiThread(new Runnable() {
                        public void run() {
                            if(Constants.PROGRESS_INDICATOR_ENABLED) {
                                mActivity.setProgressBarIndeterminateVisibility(false);
                            }
                            mInstallError = true;
                        }
                    });
                    break;
                default:
                    Log.e(Constants.LOG_TAG,
                            "Error: unknow progress state received from SPPA.");
                    mFinishedConnect = false;
                    mPendingInstall = false;
                    mActivity.runOnUiThread(new Runnable() {
                        public void run() {
                            if(Constants.PROGRESS_INDICATOR_ENABLED) {
                                mActivity.setProgressBarIndeterminateVisibility(false);
                            }
                            mInstallError = true;                        
                        }
                    });
                    break;
            }
            if(mInstallError) {
                   t=(TextView)mActivity.findViewById(R.id.provisioningText); 
                   t.setText(R.string.provisioning_ko_text);
                   t.setTextColor(Color.RED);
            }
        }
    }

    // Provision.
    public Provision(Activity activity) {
        // Disable other buttons
        Button b;

         if(activity == null) {
            Log.e(Constants.LOG_TAG, "Error: activity is null");
            return;
        }

        // Read SPApp info resources.
        if(readInfo(activity) == false) {
            return;
        }

        // Create SPPA.
        mSPPA = new SPPA(activity, mEnrollmentUrl, mSpKey);
        if(mSPPA == null) {
            Log.e(Constants.LOG_TAG, "Error: creating SPPA failed.");
            return;
        }

        mActivity = activity;

        // Create SPPA service for SPApp.
        mService = new Service(mSPPA);
        if(mService == null) {
            Log.e(Constants.LOG_TAG, "Error: creating SPPA Service failed.");
            mSPPA = null;
            return;
        }

        // Create SPPA callback for SPApp.
        mCb = new ProvisionCallback();

        // Connect to SPPA.
        if(!mSPPA.connect(mCb)) {
            Log.e(Constants.LOG_TAG, "Error: SPPA connection failed.");
            mSPPA = null;
            mService = null;
            mCb = null;
           return;
        }
    }

    // Read SPApp info resources.
    private boolean readInfo(Activity act) {
        String r;

        // Read enrollment URL.
        r = act.getResources().getString(R.string.enrollment_url);
        if(r == null) {
            Log.e(Constants.LOG_TAG,
                    "Error: failed to read enrollment_url resource.");
            return false;
        }
        mEnrollmentUrl = r;

        // Read SP key.
        r = act.getResources().getString(R.string.sp_key);
        if(r == null) {
            Log.e(Constants.LOG_TAG, "Error: failed to read sp_key resource.");
            return false;
        }
        mSpKey = Utils.resourceToBytes(r);
        if(mSpKey == null) {
            Log.e(Constants.LOG_TAG,
                    "Error: failed to parse sp_key resource.");
            return false;
        }

        // Read TA key.
        r = act.getResources().getString(R.string.ta_key);
        if(r == null) {
            Log.e(Constants.LOG_TAG, "Error: failed to read ta_key resource.");
            return false;
        }
        mKey = Utils.resourceToBytes(r);
        if(mKey == null) {
            Log.e(Constants.LOG_TAG,
                    "Error: failed to parse ta_key resource.");
            return false;
        }

        return true;
    }

    // Notice SPPA that SPApp is terminating.
    public void exit() {
        if(mSPPA == null) {
            return;
        }

        if(!mSPPA.disconnect()) {
            Log.e(Constants.LOG_TAG, "Error: SPPA disconnection failed.");
        }

        mSPPA.exit();
    }

    // Install containers required by SPApp.
     public boolean install(int uuid, int TAbutton) {
        String r;
        
        // Read Trusted Application (TA) uuid.
        if(mActivity == null)
        {
            Log.e(Constants.LOG_TAG, "Error: Activity not provisionned.");
            return false;
        }
        r = mActivity.getResources().getString(uuid);
        if(r == null) {
            Log.e(Constants.LOG_TAG, "Error: failed to read uuid resource.");
            return false;
        }
        sUuid = r;
        
        mUuid = Utils.resourceToBytes(r);
        if(mUuid == null) {
            Log.e(Constants.LOG_TAG, "Error: failed to parse uuid resource.");
            return false;
        }

        if((mService== null) ) {
            Log.e(Constants.LOG_TAG, "Error: can't access to SPPA service.");
            return false;
        }

        // Wait TSdkSample to connect to SPPA.
        AsyncTask<Integer, Void, Boolean> task =
            new AsyncTask<Integer, Void, Boolean>() {
                
                protected void onPreExecute(){
                    
                    mActivity.runOnUiThread(new Runnable() {
                        public void run() {
                            if(Constants.PROGRESS_INDICATOR_ENABLED) {
                                mActivity.setProgressBarIndeterminateVisibility(true);
                            }
                        }
                    });
                }

                protected Boolean doInBackground(Integer... params) {
                    for(int i = 0; i < WAIT_TRY; i++) {
                        if(mFinishedConnect) {
                                break;
                        } else {
                            try {
                                Thread.sleep(WAIT_PERIOD);
                            } catch (Exception e) {
                                return mFinishedConnect;
                            }
                            mPendingInstallButton = params[0];
                        }
                    }
                    return mFinishedConnect;
                }

                protected void onPostExecute(Boolean ret) {
                    if(ret == false) {
                        Log.e(Constants.LOG_TAG,
                                "Error: wait SPPA connection failed.");
                        mPendingInstall = false;
                        mActivity.runOnUiThread(new Runnable() {
                            public void run() {
                                if(Constants.PROGRESS_INDICATOR_ENABLED) {
                                    mActivity.setProgressBarIndeterminateVisibility(false);
                                }
                            }
                        });

                        return;
                    }
                    // Install.
                    mService.install(mCb, mUuid, mKey);
                }

        };

        task.execute(TAbutton);
        
        return true;
    }

    // Uninstall TA container of SPApp.
    public void uninstall() {
        Log.d(Constants.LOG_TAG, "uninstall.");

        if(mService == null) {
            Log.e(Constants.LOG_TAG, "Error: can't access to SPPA service.");
        }

        //Uninstall.
        mActivity.runOnUiThread(new Runnable() {
            public void run() {
                if(Constants.PROGRESS_INDICATOR_ENABLED) {
                    mActivity.setProgressBarIndeterminateVisibility(true);
                }
            }
        });
        mService.uninstall(mCb, mUuid);
    }

    // Get SPID of SPApp.
    public int spid() {
        if(mService == null) {
            Log.e(Constants.LOG_TAG,
                    "Error: can't access to SPPA service.");
        }
        return mService.queryAppInfo().spid();
    }
}

