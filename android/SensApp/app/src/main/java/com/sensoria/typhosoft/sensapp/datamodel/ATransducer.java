package com.sensoria.typhosoft.sensapp.datamodel;

import android.support.annotation.NonNull;

import com.sensoria.typhosoft.sensapp.core.ISensAdapterItems;

import java.io.Serializable;

/**
 * Created by santonocitom on 29/06/17.
 */

public abstract class ATransducer implements Serializable, Cloneable, ISensAdapterItems {
    protected final String name;
    protected final ESensType type;
    protected final ESensStereotype stereotype;
    protected String description;
    protected String descriptor;

    public ATransducer(String name, ESensType type, ESensStereotype stereotype, String description) {
        this.name = name;
        this.type = type;
        this.stereotype = stereotype;
        this.description = description;
    }

    public ESensType getType() {
        return type;
    }

    public String getName() {
        return name;
    }

    public String getDescriptor() {
        return descriptor;
    }

    public ESensStereotype getStereoType() {
        return stereotype;
    }

    @Override
    public int compareTo(@NonNull Object t) {
        if (t instanceof ATransducer)
            return this.name.compareTo(((ATransducer) t).getName());
        else if (t instanceof Node)
            return this.name.compareTo(((Node) t).getName());

        return 0;
    }

    @Override
    public int hashCode() {
        return this.name.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof ATransducer)
            return this.name.equalsIgnoreCase(((ATransducer) obj).name);

        return false;
    }

    @Override
    public ATransducer clone() throws CloneNotSupportedException {
        return (ATransducer) super.clone();
    }

    public abstract void read(String sentence);
}
