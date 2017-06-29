package com.sensoria.typhosoft.sensapp.data;

import com.sensoria.typhosoft.sensapp.data.SensStereotypeEnum;
import com.sensoria.typhosoft.sensapp.data.SensorTypeEnum;

import java.io.Serializable;

/**
 * Created by santonocitom on 27/06/17.
 */

public class Sensor extends ASensor{
    private String data;

    public Sensor(String command) {
        super(SensorTypeEnum.SENSOR);
        parse(command);
    }

    protected void parse(String command) {
        super.parse(command);
    }

    public String getData() {
        return data;
    }

    public void setData(String data) {
        this.data = data;
    }
}
