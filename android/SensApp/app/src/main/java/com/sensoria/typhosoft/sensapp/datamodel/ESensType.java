package com.sensoria.typhosoft.sensapp.datamodel;

/**
 * Created by santonocitom on 29/06/17.
 */

public enum ESensType {
    ACTUATOR("A", "Actuator"),
    SENSOR("S", "Sensor"),
    NODE("N", "Node");

    private String stringType;
    private String description;

    ESensType(String type, String description) {
        this.stringType = type;
        this.description = description;
    }

    public static ESensType convert(String type){
        for (ESensType t: ESensType.values()) {
            if(t.stringType.equalsIgnoreCase(type))
                return t;
        }
        return null;
    }

    public String getDescription() {
        return description;
    }
}
