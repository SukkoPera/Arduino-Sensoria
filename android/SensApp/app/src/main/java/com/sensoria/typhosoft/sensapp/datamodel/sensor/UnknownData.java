package com.sensoria.typhosoft.sensapp.datamodel.sensor;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.sensoria.typhosoft.sensapp.R;
import com.sensoria.typhosoft.sensapp.datamodel.ESensStereotype;
import com.sensoria.typhosoft.sensapp.network.SensClient;

/**
 * Created by santonocitom on 15/01/18.
 */

public class UnknownData extends Sensor {

    private TextView typeText;
    private TextView nameText;
    private TextView descriptionText;
    private boolean toBind = true;
    private View view;

    public UnknownData(String name, String description) {
        super(name, ESensStereotype.UNKNOWN, description);
    }

    @Override
    public void read(String sentence) {

    }

    @Override
    public View getView(LayoutInflater inflater, View convertView, ViewGroup parent) {
        if (view == null || convertView == null) {
            view = convertView;
            if (convertView == null) {
                view =  inflater.inflate(R.layout.sensadapter_s_item_layout, parent, false);
            toBind = true;
        }
    }

        if (toBind) {
        toBind = false;
            typeText = (TextView) view.findViewById(R.id.textRow1);
            nameText = (TextView) view.findViewById(R.id.textRow21);
            descriptionText = (TextView) view.findViewById(R.id.textRow22);
        }

        typeText.setText(ESensStereotype.UNKNOWN.name().concat(" ").concat(type.getDescription()));
        nameText.setText(name);
        descriptionText.setText(description);

        return view;
    }
}
