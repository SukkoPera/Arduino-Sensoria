package com.sensoria.typhosoft.sensapp.datamodel;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by santonocitom on 29/06/17.
 */

public class Node extends Transducer {

    protected List<Transducer> transducers = new ArrayList<>();

    public List<Transducer> getTransducers() {
        return transducers;
    }

    public void setTransducers(List<Transducer> transducers) {
        this.transducers = transducers;
    }

    public Node(String string) {
        this();
        //parse(string);
    }

    public Node() {
        super(SensorTypeEnum.NODE);
        this.stereoType = SensStereotypeEnum.NONE;
    }

    protected void parse(String command){
        //super.parse(command);
    }
}