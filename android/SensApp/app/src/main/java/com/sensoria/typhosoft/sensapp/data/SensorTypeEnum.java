package com.sensoria.typhosoft.sensapp.data;

/**
 * Created by santonocitom on 29/06/17.
 */

public enum SensorTypeEnum {
    ACTUATOR("A", "Actuator"),
    NODE("N", "Node"),
    SENSOR("S", "Sensor"),
    TRANSDUCER("T", "Transducer");

    private String stringType;
    private String description;

    SensorTypeEnum(String type, String description) {
        this.stringType = type;
        this.description = description;
    }

    public static SensorTypeEnum convert(String type){
        for (SensorTypeEnum t:SensorTypeEnum.values()) {
            if(t.stringType.equalsIgnoreCase(type))
                return t;
        }
        return null;
    }

    public String getDescription() {
        return description;
    }
}
