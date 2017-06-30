package com.sensoria.typhosoft.sensapp.data;

/**
 * Created by santonocitom on 29/06/17.
 */

public class Node extends Transducer {

    public Node(String string) {
        super(SensorTypeEnum.NODE);
        parse(string);
    }

    protected void parse(String command){
        super.parse(command);
    }
}