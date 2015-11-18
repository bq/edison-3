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

package com.trustonic.samplepinpad;

import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class PinpadHomeActivity extends Activity {
    private static final String TAG = PinpadHomeActivity.class.getSimpleName();

    private Button              launcher;
    private PinpadTLCTask       pinpadTLCTask;
    public Context              ctxt;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ctxt = this;
        getActionBar().setTitle(R.string.home_actionbar_title);
        getActionBar().setSubtitle(R.string.home_actionbar_subtitle);
        getActionBar().setIcon(R.drawable.t_logo);
        setContentView(R.layout.activity_start_tui_pinpad);
        launcher = (Button) findViewById(R.id.home_main_button);
        launcher.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                pinpadTLCTask = new PinpadTLCTask(ctxt);
                pinpadTLCTask.execute();
            }
        });
    }
}
