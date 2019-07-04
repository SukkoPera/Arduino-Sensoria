package com.sensoria.typhosoft.sensapp.datamodel.sensor;

import com.sensoria.typhosoft.sensapp.datamodel.ATransducer;
import com.sensoria.typhosoft.sensapp.datamodel.ESensStereotype;
import com.sensoria.typhosoft.sensapp.datamodel.ESensType;

/**
 * Created by santonocitom on 27/06/17.
 */

public abstract class Sensor extends ATransducer {
    private String data;

    public Sensor(String name, ESensStereotype stereotype, String description) {
        super(name, ESensType.SENSOR, stereotype, description);
    }

    public String getData() {
        return data;
    }

    public void setData(String data) {
        this.data = data;
    }
}
