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

import android.os.AsyncTask;
import android.util.Log;

import com.trustonic.tbase.ota.sppa.ifc.AppInfo;
import com.trustonic.tbase.ota.sppa.ifc.ISPPAService;
import com.trustonic.tbase.ota.sppa.ifc.ISPPAService.ProgressState;
import com.trustonic.tbase.ota.sppa.ifc.SPPA;

// Class implementing Service Provider Application (SPApp) operation with the
// Service Provider Provisioning Agent (SPPA).
public class Service implements ISPPAService {
    private SPPA mSPPA = null;
    AsyncTask<Void, Void, Boolean> mTask = null;
    byte[] mUuid = null;
    byte[] mKey = null;
    ServiceCallback mServiceCb = null;
    ProgressCallback mProvisionCb = null;
    boolean result = true;

    // SPPA callback for SPApp.
    public class ServiceCallback implements ProgressCallback {
        public void progress(ProgressState state) {
            if(mProvisionCb == null) {
                Log.d(Constants.LOG_TAG,
                        "Service received from SPPA progress state " +
                        state + ".");
            }

            switch(state) {
                case CONNECTED_ROOTPA:
                    if(mProvisionCb != null) mProvisionCb.progress(state);
                    break;
                case CONNECTING_SE:
                    if(mProvisionCb != null) mProvisionCb.progress(state);
                    break;
                case CREATING_ROOT_CONTAINER:
                    if(mProvisionCb != null) mProvisionCb.progress(state);
                    break;
                case CREATING_SP_CONTAINER:
                    if(mProvisionCb != null) mProvisionCb.progress(state);
                    break;
                case FINISHED_PROVISIONING:
                    if(mProvisionCb != null) mProvisionCb.progress(state);
                    // Continue with Trusted Application (TA) container
                    // installation.
                    installTAContainer();
                    break;
                case FINISHED_SE_INSTALL:
                    if(mProvisionCb != null) mProvisionCb.progress(state);
                    break;
                case ERROR_ROOTPA:
                    if(mProvisionCb != null) mProvisionCb.progress(state);
                    mTask = null;
                    break;
                case CONNECTING_SM:
                    if(mProvisionCb != null) mProvisionCb.progress(state);
                    break;
                case EXECUTE_SM:
                    if(mProvisionCb != null) mProvisionCb.progress(state);
                    break;
                case FINISHED_SM:
                    if(mProvisionCb != null) mProvisionCb.progress(state);
                    mTask = null;
                    break;
                case ERROR_SPPA:
                    if(mProvisionCb != null) mProvisionCb.progress(state);
                    mTask = null;
                    break;
                default:
                    Log.e(Constants.LOG_TAG,
                            "Error: unknow progress state received from SPPA.");
                    mTask = null;
            }
        }
    }

    // Service.
    public Service(SPPA sppa) {
        mSPPA = sppa;

        mServiceCb = new ServiceCallback();
    }

    // Return SPApp info from SPPA.
    public AppInfo queryAppInfo() {
        if(mSPPA == null || !mSPPA.isReady()) {
            Log.e(Constants.LOG_TAG, "Error: can't access to SPPA service.");
            return null;
        }

        return mSPPA.queryAppInfo();
    }

    // Intall TA container.
    private void installTAContainer() {
        Log.d(Constants.LOG_TAG, "installTAContainer");

        if(!mSPPA.isReady()) {
            Log.e(Constants.LOG_TAG, "Error: can't access to SPPA service.");
            return;
        }

        mServiceCb.progress(ProgressState.CONNECTING_SM);

        AsyncTask<Void, Void, Boolean> task =
            new AsyncTask<Void, Void, Boolean>() {
                protected Boolean doInBackground(Void... params) {
                    return mSPPA.installTAContainer(mUuid, mKey);
                }

                protected void onPostExecute(Boolean ret) {
                    if(ret == false) {
                        Log.e(Constants.LOG_TAG,
                                "Error: installTAContainer failed.");
                        mServiceCb.progress(ProgressState.ERROR_SPPA);
                    } else {
                         mServiceCb.progress(ProgressState.FINISHED_SM);
                    }
                }

                protected void onPreExecute() {
                    mServiceCb.progress(ProgressState.EXECUTE_SM);
                }
            };

        task.execute();
    }

    // Install SPApp TA.
    public void install(ProgressCallback cb, byte[] uuid, byte[] key) {
        Log.d(Constants.LOG_TAG, "install.");

        if(mSPPA == null || !mSPPA.isReady()) {
            Log.e(Constants.LOG_TAG, "Error: can't access to SPPA service.");
           return;
        }

        if((mTask != null)) {
            Log.e(Constants.LOG_TAG, "Error: SPPA service is busy.");
           return;
        }

        mUuid = uuid;
        mKey = key;
        mProvisionCb = cb;

        mTask =
            new AsyncTask<Void, Void, Boolean>() {
                protected Boolean doInBackground(Void... params) {
                    // Provision SPApp Root and SP containers.
                    return mSPPA.provisionContainers(mServiceCb);
                }

                protected void onPostExecute(Boolean ret) {
                        Service.this.result = ret; 
                        return;
                }

                protected void onPreExecute() {}
            };

        mTask.execute();
    }

    // Uninstall SPApp TA.
    public void uninstall(ProgressCallback cb, byte[] uuid) {
        Log.d(Constants.LOG_TAG, "uninstall.");

        if(mSPPA == null || !mSPPA.isReady()) {
            Log.e(Constants.LOG_TAG, "Error: can't access to SPPA service.");
            return;
        }

        if(mTask != null) {
            Log.e(Constants.LOG_TAG, "Error: SPPA service is busy.");
            return;
        }

        mUuid = uuid;
        mProvisionCb = cb;

        mServiceCb.progress(ProgressState.CONNECTING_SM);

        mTask =
            new AsyncTask<Void, Void, Boolean>() {
                protected Boolean doInBackground(Void... params) {
                    // Uninstall TA container.
                    return mSPPA.uninstallTAContainer(mUuid);
                }

                protected void onPostExecute(Boolean ret) {
                    if(ret == false) {
                        Log.e(Constants.LOG_TAG,
                                "Error: uninstallTAContainer failed.");
                        mServiceCb.progress(ProgressState.ERROR_SPPA);
                    } else {
                        mServiceCb.progress(ProgressState.FINISHED_SM);
                    }
                }

                protected void onPreExecute() {
                    mServiceCb.progress(ProgressState.EXECUTE_SM);
                }
            };

        mTask.execute();
    }
}

