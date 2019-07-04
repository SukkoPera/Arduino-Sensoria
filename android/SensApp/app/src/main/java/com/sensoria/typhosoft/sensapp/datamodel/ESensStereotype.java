package com.sensoria.typhosoft.sensapp.datamodel;

/**
 * Created by santonocitom on 29/06/17.
 */

public enum ESensStereotype {
    UNKNOWN("UNKNOWN"),
    NODE("Server"),
    WEATHER_DATA("WD"),
    RELAY_DATA("RS"),
    CONTROLLED_RELAY_DATA("CR"),
    MOTION_DATA("MD"),
    TIME_CONTROL_DATA("TC"),
    DATE_TIME_DATA("DT"),
    VALUE_SET_DATA("VS");

    private String stringType;

    ESensStereotype(String stringType) {
        this.stringType = stringType;
    }

    public String getStringType() {
        return stringType;
    }

    public static ESensStereotype convert(String type){
        for (ESensStereotype t: ESensStereotype.values()) {
            if(t.stringType.equalsIgnoreCase(type))
                return t;
        }
        return null;
    }
}
