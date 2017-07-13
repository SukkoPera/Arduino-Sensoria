package com.sensoria.typhosoft.sensapp.datamodel;

/**
 * Created by santonocitom on 29/06/17.
 */

public class Actuator extends Transducer {

    private Boolean onOff;
    private Boolean autoManual;

    public Actuator(String string) {
        super(SensorTypeEnum.ACTUATOR);
        parse(string);
    }

    protected void parse(String command){
        super.parse(command);
        onOff = false;
        autoManual = false;
    }

    public Boolean getOnOff() {
        return onOff;
    }

    public void setOnOff(Boolean onOff) {
        this.onOff = onOff;
    }

    public boolean getAutoManual() {
        return autoManual;
    }

    public void setAutoManual(Boolean autoManual) {
        this.autoManual = autoManual;
    }
}
