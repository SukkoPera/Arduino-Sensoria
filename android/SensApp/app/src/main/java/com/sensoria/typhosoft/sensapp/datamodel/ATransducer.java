package com.sensoria.typhosoft.sensapp.datamodel;

import android.support.annotation.NonNull;

import java.io.Serializable;

/**
 * Created by santonocitom on 29/06/17.
 */

public abstract class Transducer implements Serializable, Comparable<Transducer> {
    protected String name;
    protected SensorTypeEnum type;
    protected String descriptor;
    protected SensStereotypeEnum stereoType;

    public Transducer(SensorTypeEnum type) {
        this.type = type;
    }

    public SensorTypeEnum getType() {
        return type;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getDescriptor() {
        return descriptor;
    }

    protected void parse(String command) {
        if (command.length() >= 2) {
            name = command.substring(0, 2);
        }

        if (command.length() >= 7) {
            stereoType = SensStereotypeEnum.convert(command.substring(5, 7));
        }
        if (command.length() >= 8) {
            descriptor = command.substring(8);
        }
    }

    public SensStereotypeEnum getStereoType() {
        return stereoType;
    }

    @Override
    public int compareTo(@NonNull Transducer t) {
        return this.name.compareTo(t.getName());
    }

    @Override
    public int hashCode() {
        return this.name.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof Transducer)
            return this.name.equalsIgnoreCase(((Transducer) obj).name);

        return false;
    }
}
