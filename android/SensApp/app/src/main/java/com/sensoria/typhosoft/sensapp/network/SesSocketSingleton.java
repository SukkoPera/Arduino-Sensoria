package com.sensoria.typhosoft.sensapp.network;

import java.net.DatagramSocket;
import java.net.SocketException;

class SesSocketSingleton {

    static public int UdpPort = 9999;
    private DatagramSocket listenerSocket = null;

    private static final SesSocketSingleton ourInstance = new SesSocketSingleton();

    static SesSocketSingleton getInstance() {
        return ourInstance;
    }

    private SesSocketSingleton() {
        try {
            connect();
        } catch (SocketException e) {
            e.printStackTrace();
        }
    }

    private void connect() throws SocketException {
        setListenerSocket(new DatagramSocket(UdpPort));
        getListenerSocket().setBroadcast(true);
        getListenerSocket().setReuseAddress(true);
    }

    public void exit() {
        if (getListenerSocket() != null) {
            getListenerSocket().disconnect();
            getListenerSocket().close();
        }
    }

    public DatagramSocket getListenerSocket() {
        return listenerSocket;
    }

    private void setListenerSocket(DatagramSocket listenerSocket) {
        this.listenerSocket = listenerSocket;
    }
}
