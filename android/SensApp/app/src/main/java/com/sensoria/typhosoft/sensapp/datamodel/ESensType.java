package com.sensoria.typhosoft.sensapp.datamodel;

/**
 * Created by santonocitom on 29/06/17.
 */

public enum ESensorType {
    ACTUATOR("A", "Actuator"),
    SENSOR("S", "Sensor"),
    NODE("N", "Node");

    private String stringType;
    private String description;

    ESensorType(String type, String description) {
        this.stringType = type;
        this.description = description;
    }

    public static ESensorType convert(String type){
        for (ESensorType t: ESensorType.values()) {
            if(t.stringType.equalsIgnoreCase(type))
                return t;
        }
        return null;
    }

    public String getDescription() {
        return description;
    }
}
