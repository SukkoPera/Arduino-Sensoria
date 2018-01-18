package com.sensoria.typhosoft.sensapp.datamodel.sensor;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.sensoria.typhosoft.sensapp.R;
import com.sensoria.typhosoft.sensapp.datamodel.ESensStereotype;
import com.sensoria.typhosoft.sensapp.datamodel.actuator.Actuator;

/**
 * Created by santonocitom on 15/01/18.
 */

public class TimerData extends Sensor {

    private TextView typeText;
    private TextView nameText;
    private TextView descriptionText;

    public TimerData(String name, String description) {
        super(name, ESensStereotype.TIME_CONTROL_DATA, description);
    }

    @Override
    public void read(String sentence) {

    }

    @Override
    public View getView(LayoutInflater inflater, View convertView, ViewGroup parent) {
        View vi = convertView;
        if (vi == null) {
            vi = inflater.inflate(R.layout.sensadapter_s_item_layout, parent, false);
            typeText = (TextView) vi.findViewById(R.id.textRow1);
            nameText = (TextView) vi.findViewById(R.id.textRow21);
            descriptionText = (TextView) vi.findViewById(R.id.textRow22);
        }

        typeText.setText(type.getDescription());
        nameText.setText(name);
        descriptionText.setText(description);

        return vi;
    }
}
