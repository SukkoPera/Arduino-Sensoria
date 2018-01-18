package com.sensoria.typhosoft.sensapp.custom.adapter;

import com.sensoria.typhosoft.sensapp.datamodel.ESensStereotype;
import com.sensoria.typhosoft.sensapp.datamodel.ESensType;

/**
 * Created by santonocitom on 15/01/18.
 */

public interface ISensAdapterItems {

    ESensType getType();
    ESensStereotype getStereoType();
    String getName();
    String getDescriptor();
}
