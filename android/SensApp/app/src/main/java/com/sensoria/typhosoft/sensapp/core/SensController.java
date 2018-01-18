package com.sensoria.typhosoft.sensapp.datamodel;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Created by santonocitom on 29/06/17.
 */

public class SensModel {
    private static final SensModel ourInstance = new SensModel();
    private final ArrayList<Node> items = new ArrayList<>();
    private Lock lock = new ReentrantLock();

    private SensModel() {
    }

    public static SensModel getInstance() {
        return ourInstance;
    }

    public List<Node> getItems() {
        List<Node> nodes = null;
        lock.lock();
        try {
            nodes = Collections.unmodifiableList(items);
        } finally {
            lock.unlock();
        }
        return nodes;
    }

    private ATransducer findItemsByName(String name) {
        ATransducer transducer = null;
        lock.lock();
        try {
            for (Node node : items) {
                for (ATransducer sensor : node.getTransducers()) {
                    if (sensor.getName().equalsIgnoreCase(name)) {
                        transducer = sensor;
                        break;
                    }
                    if (transducer != null) break;
                }
            }
        } finally {
            lock.unlock();
        }
        return transducer;
    }

    public ATransducer getItemsByName(String name) {
        ATransducer transducer = null;
        try {
            transducer = findItemsByName(name).clone();
        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }
        return transducer;
    }

    public void addUpdateNode(Node node) {
        lock.lock();
        try {
            if (!items.contains(node)) {
                items.add(node);
            }
        } finally {
            lock.unlock();
        }
    }

    public void read(String name, String sentence) {
        findItemsByName(name).read(sentence);
    }
}
