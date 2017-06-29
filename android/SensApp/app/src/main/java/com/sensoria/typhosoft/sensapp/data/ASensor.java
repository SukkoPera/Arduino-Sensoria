package com.sensoria.typhosoft.sensapp.data;

import java.io.Serializable;

/**
 * Created by santonocitom on 29/06/17.
 */

public abstract class ASensor implements Serializable {
    private String name;
    private SensorTypeEnum type;
    private String descriptor;
    private SensStereotypeEnum stereoType;

    public ASensor(SensorTypeEnum type) {
        this.type = type;
    }

    public SensorTypeEnum getType(){
        return type;
    }

    public String getName(){
        return name;
    }

    public String getDescriptor() {
        return descriptor;
    }

    protected void parse(String command) {
        name = command.substring(0,2);
        descriptor = command.substring(8);
    }
    public SensStereotypeEnum getStereoType() {
        return stereoType;
    }
}
