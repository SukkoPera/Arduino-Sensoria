package com.sensoria.typhosoft.sensapp.data;

import java.io.Serializable;

/**
 * Created by santonocitom on 29/06/17.
 */

public abstract class Transducer implements Serializable {
    private String name;
    private SensorTypeEnum type;
    private String descriptor;
    private SensStereotypeEnum stereoType;

    public Transducer(SensorTypeEnum type) {
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
        stereoType = SensStereotypeEnum.convert(command.substring(5,7));
        descriptor = command.substring(8);
    }
    public SensStereotypeEnum getStereoType() {
        return stereoType;
    }
}
