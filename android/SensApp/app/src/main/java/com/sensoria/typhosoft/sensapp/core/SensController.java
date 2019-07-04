package com.sensoria.typhosoft.sensapp.core;

import com.sensoria.typhosoft.sensapp.datamodel.ATransducer;
import com.sensoria.typhosoft.sensapp.datamodel.Node;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Created by santonocitom on 29/06/17.
 */

public class SensController {
    private static final SensController ourInstance = new SensController();
    protected final ArrayList<ISensAdapterItems> adapterItems = new ArrayList<>();
    private final ArrayList<Node> items = new ArrayList<>();
    private Lock lock = new ReentrantLock();

    private SensController() {
    }

    public static SensController getInstance() {
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
                adapterItems.add(node);
                adapterItems.addAll(node.getTransducers());
            } else {
                Node oldNode = items.get(items.indexOf(node));
                if (oldNode.getTransducers().size() != node.getTransducers().size()) {
                    // TODO: WOW! you are unlucky :-(

                }
            }
        } finally {
            lock.unlock();
        }
    }

    public void read(String name, String sentence) {
        ATransducer item = findItemsByName(name);
        if (item != null)
            item.read(sentence);
        else
            System.out.println("unable find: " + name);
    }
}
