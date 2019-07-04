package com.sensoria.typhosoft.sensapp.datamodel.actuator;

import com.sensoria.typhosoft.sensapp.datamodel.ATransducer;
import com.sensoria.typhosoft.sensapp.datamodel.ESensStereotype;
import com.sensoria.typhosoft.sensapp.datamodel.ESensType;

/**
 * Created by santonocitom on 29/06/17.
 */

public abstract class Actuator extends ATransducer {

    public Actuator(String name, ESensStereotype stereotype, String description) {
        super(name, ESensType.ACTUATOR, stereotype, description);
    }

    public abstract String write();
}
