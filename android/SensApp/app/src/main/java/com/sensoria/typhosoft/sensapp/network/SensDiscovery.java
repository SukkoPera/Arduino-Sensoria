package com.sensoria.typhosoft.sensapp.network;

import android.widget.CheckBox;

/**
 * Created by santonocitom on 17/01/18.
 */

public class SensDiscovery extends Thread {
    private boolean isFinish = false;
    private CheckBox autoDiscovery;
    private boolean isPause = false;

    public SensDiscovery(CheckBox autoDiscovery) {
        super(SensDiscovery.class.getSimpleName());
        this.autoDiscovery = autoDiscovery;
        setDaemon(true);
    }

    @Override
    public void run() {
        while (!isFinish) {
            if (!isPause) {
                SesSocketSingleton.getInstance().sendMessage("HLO");
            }
            try {
                Thread.sleep(10000);
            } catch (InterruptedException e) {
                System.out.println("Discovery Thread is stopped!");
            } finally {

            }
        }
    }

    public void exit() {
        isFinish = true;
        interrupt();
    }

    public void pause() {
        isPause = !isPause;
    }

    public void OnPause() {
        isPause = true;
    }
    public void OnResume() {
        isPause = !autoDiscovery.isChecked();
    }

}
