package com.sensoria.typhosoft.sensapp;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InterfaceAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Enumeration;

public class MainActivity extends AppCompatActivity {

    private ArrayList<String> items = new ArrayList();
    private UDPListener listener;
    private int UdpPort = 9999;
    private EditText ip;
    private EditText cmd;
    ListView list;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ip = (EditText) findViewById(R.id.editTextIP);
        cmd = (EditText) findViewById(R.id.editTextCMD);

        ArrayAdapter<String> adapter =
                new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, items);

        list = (ListView) findViewById(R.id.list);
        list.setAdapter(adapter);

        listener = new UDPListener();
        listener.start();

        ip.setText(getBroadcast());
    }

    public void sendUdpPacket(View view) {
        items.clear();
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ((ArrayAdapter)list.getAdapter()).notifyDataSetChanged();
            }
        });
        listener.sendMessage(cmd.getText().toString() + "\n");
    }

    private String getBroadcast() {
        try {
            for (Enumeration<NetworkInterface> niEnum = NetworkInterface.getNetworkInterfaces(); niEnum.hasMoreElements(); ) {
                NetworkInterface ni = niEnum.nextElement();
                if (!ni.isLoopback()) {
                    for (InterfaceAddress interfaceAddress : ni.getInterfaceAddresses()) {
                        if (interfaceAddress.getBroadcast() != null) {
                            return interfaceAddress.getBroadcast().toString().substring(1);
                        }
                    }
                }
            }
        } catch (SocketException e) {
            e.printStackTrace();
        }
        return "Error";
    }

    public static boolean isThisMyIpAddress(InetAddress addr) {
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

    private class UDPListener extends Thread {

        private DatagramSocket listenerSocket = null;

        public UDPListener() {
            super("SensApp UDP Listener");
        }


        public void sendMessage(String message) {
            try {
                byte[] buf = message.getBytes();

                DatagramPacket packet = null;

                packet = new DatagramPacket(buf, 0, buf.length, InetAddress.getByName(ip.getText().toString()), UdpPort);
                listenerSocket.send(packet);
            } catch (Exception e) {
                System.err.println("Sending failed. " + e.getMessage());
            }
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
                    if (!items.contains(sentence) && !isThisMyIpAddress(receivePacket.getAddress())) {

                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                items.add(sentence);
                                ((ArrayAdapter)list.getAdapter()).notifyDataSetChanged();
                            }
                        });
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
    }

}
