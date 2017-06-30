package com.sensoria.typhosoft.sensapp.network;

import com.sensoria.typhosoft.sensapp.SensActivity;
import com.sensoria.typhosoft.sensapp.core.SensParser;
import com.sensoria.typhosoft.sensapp.data.Transducer;
import com.sensoria.typhosoft.sensapp.data.SensCommandEnum;
import com.sensoria.typhosoft.sensapp.data.SensModel;
import com.sensoria.typhosoft.sensapp.data.Sensor;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InterfaceAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Enumeration;
import java.util.List;

/**
 * Created by santonocitom on 26/06/17.
 */

public class SensClient extends Thread {

    private final InetAddress broadcast;
    private final SensActivity main;
    private int UdpPort = 9999;
    private DatagramSocket listenerSocket = null;

    public SensClient(SensActivity main) {
        super("SensApp client");
        this.main = main;
        broadcast = getBroadcast();

    }

    private static boolean isThisMyIpAddress(InetAddress addr) {
        // Check if the address is a valid special local or loop back
        if (addr.isAnyLocalAddress() || addr.isLoopbackAddress())
            return true;

        // Check if the address is defined on any interface
        try {
            return NetworkInterface.getByInetAddress(addr) != null;
        } catch (SocketException e) {
            return false;
        }
    }

    public void sendMessage(String message) {
        try {
            byte[] buf = message.getBytes();

            DatagramPacket packet = new DatagramPacket(buf, 0, buf.length, broadcast, UdpPort);
            listenerSocket.send(packet);
            System.out.println("Message sent: " + message);
        } catch (Exception e) {
            System.err.println("Sending failed. " + e.getMessage());
        }
    }

    private InetAddress getBroadcast() {
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

    @Override
    public void run() {
        try {
            listenerSocket = new DatagramSocket(UdpPort);
            // listenerSocket.bind(new InetSocketAddress(UdpPort));
            listenerSocket.setBroadcast(true);
            listenerSocket.setReuseAddress(true);
            byte[] receiveData = new byte[listenerSocket.getReceiveBufferSize()];
            DatagramPacket receivePacket = new DatagramPacket(receiveData,
                    receiveData.length);

            while (true) {
                listenerSocket.receive(receivePacket);
                final String sentence = new String(receivePacket.getData(), 0,
                        receivePacket.getLength());
                if (!isThisMyIpAddress(receivePacket.getAddress())) {
                    parse(clean(sentence));
                }
            }
        } catch (SocketException e) {
            e.printStackTrace();
        } catch (UnknownHostException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private String clean(String sentence) {
        return sentence.trim();
    }

    private synchronized void parse(String sentence) {

        SensCommandEnum sensCmd = SensParser.parseCommand(sentence);
        if (sensCmd != null) {
            switch (sensCmd) {
                case QRY:
                    SensModel.getInstance().getItems().clear();
                    SensModel.getInstance().getItems().addAll(SensParser.makeSensors(sentence));
                    requestREA(SensModel.getInstance().getItems());
                    break;
                case REA:
                    SensParser.parseREA(sentence);
                    break;
                default:
                    SensModel.getInstance().getItems().clear();
                    SensModel.getInstance().getItems().add(new Sensor(sentence));
            }
            main.updateSens();
        }
    }

    private void requestREA(List<Transducer> retVal) {
        for (Transducer sensor : retVal) {
            sendMessage("REA " + sensor.getName());
        }
    }

}