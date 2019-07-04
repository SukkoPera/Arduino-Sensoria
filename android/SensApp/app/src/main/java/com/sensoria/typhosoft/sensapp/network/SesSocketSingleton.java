package com.sensoria.typhosoft.sensapp.network;

import android.os.AsyncTask;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InterfaceAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

public class SesSocketSingleton {

    private static final SesSocketSingleton ourInstance = new SesSocketSingleton();
    static public int UdpPort = 9999;
    private DatagramSocket listenerSocket = null;
    private DatagramSocket sendSocket = null;

    private SesSocketSingleton() {
        try {
            connect();
        } catch (SocketException e) {
            e.printStackTrace();
        }
    }

    public static SesSocketSingleton getInstance() {
        return ourInstance;
    }

    public void sendMessage(String message) {
        message.concat("\r");
        byte[] buf = message.getBytes();
        InetAddress broadcast = getBroadcast();
        System.out.println("Message sent: " + message + " to: " + broadcast);
        if (broadcast == null) return;
        DatagramPacket packet = new DatagramPacket(buf, 0, buf.length, broadcast, SesSocketSingleton.UdpPort);
        new SensNetworkTask().executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, packet);
    }


    public InetAddress getBroadcast() {
        try {
            for (Enumeration<NetworkInterface> niEnum = NetworkInterface.getNetworkInterfaces(); niEnum.hasMoreElements(); ) {
                NetworkInterface ni = niEnum.nextElement();
                if (!ni.isLoopback()) {
                    for (InterfaceAddress interfaceAddress : ni.getInterfaceAddresses()) {
                        if (interfaceAddress.getBroadcast() != null) {
                            return interfaceAddress.getBroadcast();
                        }
                    }
                }
            }
        } catch (SocketException e) {
            e.printStackTrace();
        }
        return null;
    }

    private void connect() throws SocketException {
        listenerSocket = sendSocket = new DatagramSocket(UdpPort);
        sendSocket.setBroadcast(true);
        sendSocket.setReuseAddress(true);
        System.out.println("getReuseAddress() == " + sendSocket.getReuseAddress());
        System.out.println("getLocalAddress() == " + sendSocket.getLocalAddress());
        System.out.println("getLocalPort() == " + sendSocket.getLocalPort());
        System.out.println("getBroadcast() == " + sendSocket.getBroadcast());
        System.out.println("getRemoteSocketAddress() == " + sendSocket.getRemoteSocketAddress());
        System.out.println("getPort() == " + sendSocket.getPort());


    }

    public void exit() {
        if (getListenerSocket() != null) {
            getListenerSocket().disconnect();
            getListenerSocket().close();
        }
        if (getSendSocket() != null) {
            getSendSocket().disconnect();
            getSendSocket().close();
        }
    }

    public void receive(DatagramPacket message) throws IOException {
        getListenerSocket().receive(message);
    }

    public void send(DatagramPacket message) throws IOException {
        getSendSocket().send(message);
    }

    public DatagramSocket getListenerSocket() {
        return listenerSocket;
    }

    public void setListenerSocket(DatagramSocket listenerSocket) {
        this.listenerSocket = listenerSocket;
    }

    public DatagramSocket getSendSocket() {
        return sendSocket;
    }

    public void setSendSocket(DatagramSocket sendSocket) {
        this.sendSocket = sendSocket;
    }
}
