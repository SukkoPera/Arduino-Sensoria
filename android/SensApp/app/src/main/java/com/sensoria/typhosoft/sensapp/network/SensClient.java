package com.sensoria.typhosoft.sensapp.network;

import android.os.AsyncTask;

import com.sensoria.typhosoft.sensapp.SensActivity;
import com.sensoria.typhosoft.sensapp.core.SensController;
import com.sensoria.typhosoft.sensapp.core.SensNewParser;
import com.sensoria.typhosoft.sensapp.datamodel.ATransducer;
import com.sensoria.typhosoft.sensapp.datamodel.ESensCommand;
import com.sensoria.typhosoft.sensapp.datamodel.Node;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.List;

/**
 * Created by santonocitom on 26/06/17.
 */

public class SensClient extends AsyncTask<String, Integer, String> {

    private SensActivity main;
    private volatile boolean isFinish = false;

    public SensClient(SensActivity main) {
        this.main = main;
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


    private String clean(String sentence) {
        return sentence.trim();
    }

    private void parse(String sentence) {
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
            SesSocketSingleton.getInstance().sendMessage(ESensCommand.REA.getCmd().concat(" ").concat(sensor.getName()));
        }
    }

    @Override
    protected String doInBackground(String... strings) {
        try {
            byte[] receiveData = new byte[SesSocketSingleton.getInstance().getListenerSocket().getReceiveBufferSize()];
            DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
            System.out.println("Receiver started! isFinish: " + isFinish);
            while (!isFinish) {
                SesSocketSingleton.getInstance().receive(receivePacket);
                String sentence = new String(receivePacket.getData(), 0, receivePacket.getLength());
                System.out.println("Received sentence: " + sentence);
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
        return "";
    }

    public void exit() {
        isFinish = true;
    }
}