package com.sensoria.typhosoft.sensapp.core;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.sensoria.typhosoft.sensapp.datamodel.ESensStereotype;
import com.sensoria.typhosoft.sensapp.network.SensClient;

/**
 * Created by santonocitom on 15/01/18.
 */

public interface ISensAdapterItems extends Comparable {

    View getView(LayoutInflater inflater, View convertView, ViewGroup parent);

    ESensStereotype getStereoType();
}
