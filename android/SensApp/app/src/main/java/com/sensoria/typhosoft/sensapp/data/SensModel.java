package com.sensoria.typhosoft.sensapp.data;

import java.util.ArrayList;

/**
 * Created by santonocitom on 29/06/17.
 */

public class SensModel {
    private static final SensModel ourInstance = new SensModel();

    private final ArrayList<ASensor> items = new ArrayList<>();

    public static SensModel getInstance() {
        return ourInstance;
    }

    private SensModel() {
    }

    public ArrayList<ASensor> getItems() {
        return items;
    }

    public ASensor getItemsByName(String name){
        for (ASensor sensor : items) {
            if(sensor.getName().equalsIgnoreCase(name))
                return sensor;
        }
        return null;
    }
}
