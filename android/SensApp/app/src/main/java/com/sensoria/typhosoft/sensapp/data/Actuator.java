package com.sensoria.typhosoft.sensapp.data;

/**
 * Created by santonocitom on 29/06/17.
 */

public class Actuator extends ASensor {

    private Boolean onOff;

    public Actuator(String string) {
        super(SensorTypeEnum.ACTUATOR);
        parse(string);
    }

    protected void parse(String command){
        super.parse(command);
        onOff = false;


    }

    public Boolean getOnOff() {
        return onOff;
    }

    public void setOnOff(Boolean onOff) {
        this.onOff = onOff;
    }
}
