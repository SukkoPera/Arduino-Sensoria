package com.sensoria.typhosoft.sensapp.datamodel;

/**
 * Created by santonocitom on 27/06/17.
 */

public class Sensor extends ATransducer {
    private String data;

    public Sensor(String command) {
        super(ESensorType.SENSOR);
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
