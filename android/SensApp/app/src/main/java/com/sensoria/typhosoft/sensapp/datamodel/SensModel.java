package com.sensoria.typhosoft.sensapp.datamodel;

import java.util.ArrayList;

/**
 * Created by santonocitom on 29/06/17.
 */

public class SensModel {
    private static final SensModel ourInstance = new SensModel();

    private final ArrayList<Transducer> items = new ArrayList<>();

    public static SensModel getInstance() {
        return ourInstance;
    }

    private SensModel() {
    }

    public ArrayList<Transducer> getItems() {
        return items;
    }

    public Transducer getItemsByName(String name){
        for (Transducer sensor : items) {
            if(sensor.getName().equalsIgnoreCase(name))
                return sensor;
        }
        return null;
    }
}
