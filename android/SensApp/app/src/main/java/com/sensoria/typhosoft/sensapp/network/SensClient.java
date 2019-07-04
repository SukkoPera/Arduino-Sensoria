package com.sensoria.typhosoft.sensapp.network;

import com.sensoria.typhosoft.sensapp.SensActivity;
import com.sensoria.typhosoft.sensapp.core.SensController;
import com.sensoria.typhosoft.sensapp.core.SensNewParser;
import com.sensoria.typhosoft.sensapp.datamodel.ATransducer;
import com.sensoria.typhosoft.sensapp.datamodel.ESensCommand;
import com.sensoria.typhosoft.sensapp.datamodel.Node;

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
    private boolean isFinish = false;

    public SensClient(SensActivity main) {
        super("SensApp client");
        this.main = main;
        broadcast = getBroadcast();
        setDaemon(true);
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
            message.concat("\r");
            byte[] buf = message.getBytes();

            DatagramPacket packet = new DatagramPacket(buf, 0, buf.length, broadcast, SesSocketSingleton.UdpPort);

            new SensNetworkTask().execute(packet);

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
            byte[] receiveData = new byte[SesSocketSingleton.getInstance().getListenerSocket().getReceiveBufferSize()];
            DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);

            while (!isFinish) {
                SesSocketSingleton.getInstance().getListenerSocket().receive(receivePacket);
                final String sentence = new String(receivePacket.getData(), 0, receivePacket.getLength());
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
        } finally {
            System.out.println("SensClient Thread is stopped!");
        }
    }



    private String clean(String sentence) {
        return sentence.trim();
    }

    private synchronized void parse(String sentence) {
        boolean mustUpdate = true;
        ESensCommand sensCmd = SensNewParser.parseCommand(sentence);
        if (sensCmd != null) {
            switch (sensCmd) {
                case VER:
                    break;
                case HLO:
                    Node node = SensNewParser.parseHLO(sentence);
                    SensController.getInstance().addUpdateNode(node);
                    requestREA(node.getTransducers());
                    break;
                case WRI:
                    mustUpdate = false;
                    break;
                case REA:
                    String name = SensNewParser.parseTransducerName(sentence);
                    SensController.getInstance().read(name, sentence);
                    break;
                case NRQ:
                    break;
                default:
                    mustUpdate = false;
            }
            if (mustUpdate)
                main.updateSens();
        }
    }

    private void requestREA(List<ATransducer> retVal) {
        for (ATransducer sensor : retVal) {
            sendMessage(ESensCommand.REA.getCmd().concat(" ").concat(sensor.getName()));
        }
    }
}