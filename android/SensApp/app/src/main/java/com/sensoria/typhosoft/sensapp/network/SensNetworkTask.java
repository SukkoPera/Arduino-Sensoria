package com.sensoria.typhosoft.sensapp.network;

import android.os.AsyncTask;

import java.io.IOException;
import java.net.DatagramPacket;

class SensNetworkTask extends AsyncTask<DatagramPacket, Integer, String> {

    @Override
    protected String doInBackground(DatagramPacket[] messages) {

            try {
                SesSocketSingleton.getInstance().getListenerSocket().send(messages[0]);
            } catch (IOException e) {
                e.printStackTrace();
            }
            System.out.println("Message sent: " + messages.toString());

        return "";
    }
}
