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

import java.lang.Thread;

import android.app.Activity;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.Button;

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
    private String mEnrollmentUrl = null;
    private byte[] mSpKey = null;
    private byte[] mUuid = null;
    private byte[] mKey = null;

    // SPPA callback for SPApp.
    public class ProvisionCallback implements ProgressCallback {
        public void progress(ProgressState state) {
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
                            "Provisioning has finished sucessfully.");
                    break;
                case FINISHED_SE_INSTALL:
                    Log.d(Constants.LOG_TAG,
                            "Developer TA installation finished sucessfully.");
                    break;
                case ERROR_ROOTPA:
                    Log.e(Constants.LOG_TAG,
                            "Error: RootPA and/or Service Enabler operation failed.");
                    mFinishedConnect = false;
                    break;
                case CONNECTING_SM:
                    Log.d(Constants.LOG_TAG, "Connecting to Service Manager.");
                    mActivity.runOnUiThread(new Runnable() {
                        public void run() {
                            Button b;

                            b = (Button)mActivity.findViewById(R.id.rot13SWdButton);
                            b.setEnabled(false);
                            b = (Button)mActivity.findViewById(R.id.uninstallRot13SWdButton);
                            b.setEnabled(false);
                        }
                    });
                    break;
                case EXECUTE_SM:
                    Log.d(Constants.LOG_TAG, "Execute Service Manager operation.");
                    break;
                case FINISHED_SM:
                    Log.d(Constants.LOG_TAG,
                            "Service Manager operation finished sucessfully.");
                    mActivity.runOnUiThread(new Runnable() {
                        public void run() {
                            Button b;

                            b = (Button)mActivity.findViewById(R.id.rot13SWdButton);
                            b.setEnabled(mPendingInstall);
                            b = (Button)mActivity.findViewById(R.id.uninstallRot13SWdButton);
                            b.setEnabled(mPendingInstall);
                            mPendingInstall = false;
                            if(Constants.PROGRESS_INDICATOR_ENABLED) {
                                mActivity.setProgressBarIndeterminateVisibility(false);
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
                        }
                    });
                    break;
            }
        }
    }

    // Provision.
    public Provision(Activity activity) {
        // Disable button.
        Button b;

        b = (Button)activity.findViewById(R.id.rot13SWdButton);
        b.setEnabled(false);
        b = (Button)activity.findViewById(R.id.uninstallRot13SWdButton);
        b.setEnabled(false);

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
    private boolean readInfo(Activity a) {
        String r;

        // Read enrollment URL.
        r = a.getResources().getString(R.string.enrollment_url);
        if(r == null) {
            Log.e(Constants.LOG_TAG,
                    "Error: failed to read enrollment_url resource.");
            return false;
        }
        mEnrollmentUrl = r;

        // Read SP key.
        r = a.getResources().getString(R.string.sp_key);
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

        // Read Trusted Application (TA) uuid.
        r = a.getResources().getString(R.string.uuid);
        if(r == null) {
            Log.e(Constants.LOG_TAG, "Error: failed to read uuid resource.");
            return false;
        }
        mUuid = Utils.resourceToBytes(r);
        if(mUuid == null) {
            Log.e(Constants.LOG_TAG, "Error: failed to parse uuid resource.");
            return false;
        }

        // Read TA key.
        r = a.getResources().getString(R.string.ta_key);
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
    public void install() {
        Log.d(Constants.LOG_TAG, "install.");

        if((mService== null) || (mPendingInstall != false)) {
            Log.e(Constants.LOG_TAG, "Error: can't access to SPPA service.");
            return;
        }

        // Wait SPApp to connect to SPPA.
        AsyncTask<Void, Void, Boolean> task =
            new AsyncTask<Void, Void, Boolean>() {
                protected Boolean doInBackground(Void... params) {
                    for(int i = 0; i < WAIT_TRY; i++) {
                        if(mFinishedConnect) {
                            break;
                        } else {
                            try {
                                Thread.sleep(WAIT_PERIOD);
                            } catch (Exception e) {
                                return mFinishedConnect;
                            }
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

                protected void onPreExecute() {
                    mActivity.runOnUiThread(new Runnable() {
                        public void run() {
                            if(Constants.PROGRESS_INDICATOR_ENABLED) {
                                mActivity.setProgressBarIndeterminateVisibility(true);
                            }
                        }
                    });
                    mPendingInstall = true;
                }
            };

        task.execute();
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

    // Get TA filename of SPApp.
    public String taName() {
        return Utils.bytesToString(mUuid, Utils.StringType.NAME) + ".tlbin";
    }
}
