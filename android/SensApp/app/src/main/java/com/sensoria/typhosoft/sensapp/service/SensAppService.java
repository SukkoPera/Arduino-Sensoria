package com.sensoria.typhosoft.sensapp.service;

import android.app.IntentService;
import android.content.Intent;

/**
 * Created by santonocitom on 26/06/17.
 */

public class SensAppService extends IntentService {

    /**
     * Creates an IntentService.  Invoked by your subclass's constructor.
     *
     * @param name Used to name the worker thread, important only for debugging.
     */
    public SensAppService(String name) {
        super(name);
    }

    @Override
    protected void onHandleIntent(Intent workIntent) {
        // Gets data from the incoming Intent
        String dataString = workIntent.getDataString();

        // Do work here, based on the contents of dataString

    }
}
