package com.trustonic.samplepinpad;

import com.trustonic.util.tLog;

public class Screen {
    private static final String TAG             = PinpadProvision.class
            .getSimpleName();

    private int width;
    private int height;
    private int density;
    private int size;

    /** Type of resolution **/
    /* [INTERNAL]
     * Arbitrary values, we should take more generic values here.
     * Check that link: http://http://developer.android.com/guide/practices/screens_support.html#range
     * [/INTERNAL]  */
    protected static final int LOW_RES_WIDTH         = 300;
    protected static final int MEDIUM_RES_WIDTH      = 600;
    protected static final int HIGH_RES_WIDTH        = 1080;
    protected static final int EXTRA_HIGH_RES_WIDTH  = 1400;

    protected enum Type {
        UNSUPPORTED,
        LOW,
        MEDIUM,
        HIGH,
        EXTRA_HIGH
    }

    private Type type;

    Screen() {
        width = PinpadTLCWrapper.screenWidth;
        height = PinpadTLCWrapper.screenHeight;
        /* 
         * [INTERNAL] TODO-[2013-10-24]-[julare01]: We should also retrieve the 
         * density from the TA.
         * [/INTERNAL]
         */
        density = 0;
        size = 0;
        type = Type.UNSUPPORTED;
        // Define what kind of resolution are we facing.
        /* 
         * [INTERNAL] TODO-[2013-10-24]-[julare01]: We should also determine the 
         * size of the screen from the density information retrieved from the TA.
         * [/INTERNAL]
         */

        if (width < MEDIUM_RES_WIDTH) {
            type = Type.LOW;
        } else if ((width >= MEDIUM_RES_WIDTH) && (width < HIGH_RES_WIDTH)) {
            type = Type.MEDIUM;
        } else if ((width >= HIGH_RES_WIDTH) && (width < EXTRA_HIGH_RES_WIDTH)) {
            type = Type.HIGH;
        } else if (width > EXTRA_HIGH_RES_WIDTH) {
            type = Type.EXTRA_HIGH;
        } else {
            tLog.e(TAG, "Error wrong resolution!");
            type = Type.UNSUPPORTED;
        }
    }

    public Type getType() {
        return type;
    }
    
    public boolean IsSupported () {
        return type != Type.UNSUPPORTED;
    }
}
