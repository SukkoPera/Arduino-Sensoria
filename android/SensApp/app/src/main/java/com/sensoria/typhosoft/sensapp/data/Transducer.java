package com.sensoria.typhosoft.sensapp.data;

/**
 * Created by santonocitom on 29/06/17.
 */

public class Transducer extends ASensor {

    public Transducer(String string) {
        super(SensorTypeEnum.TRANSDUCER);
        parse(string);
    }

    protected void parse(String command){
        super.parse(command);
    }
}