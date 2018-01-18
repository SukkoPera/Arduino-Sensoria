package com.sensoria.typhosoft.sensapp.datamodel;

/**
 * Created by santonocitom on 29/06/17.
 */

public enum SensStereotypeEnum {
    NONE("NONE"),
    WEATHER_DATA("WD"),
    RELAY_DATA("RS"),
    CONTROLLED_RELAY_DATA("CR"),
    MOTION_DATA("MD"),
    TIME_CONTROL_DATA("TC");

    private String stringType;

    SensStereotypeEnum(String stringType) {
        this.stringType = stringType;
    }

    public String getStringType() {
        return stringType;
    }

    public static SensStereotypeEnum convert(String type){
        for (SensStereotypeEnum t:SensStereotypeEnum.values()) {
            if(t.stringType.equalsIgnoreCase(type))
                return t;
        }
        return null;
    }
}
