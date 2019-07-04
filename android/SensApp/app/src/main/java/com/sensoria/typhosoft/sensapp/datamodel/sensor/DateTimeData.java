package com.sensoria.typhosoft.sensapp.datamodel.sensor;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.sensoria.typhosoft.sensapp.R;
import com.sensoria.typhosoft.sensapp.core.SensNewParser;
import com.sensoria.typhosoft.sensapp.datamodel.ESensStereotype;
import com.sensoria.typhosoft.sensapp.network.SensClient;

import java.util.Arrays;
import java.util.Map;

/**
 * Created by santonocitom on 18/01/18.
 */

public class DateTimeData extends Sensor {
    private String data = "";
    private TextView typeText;
    private TextView nameText;
    private TextView descriptionText;
    private TextView textData;
    private boolean toBind = true;
    private View view;

    public DateTimeData(String name, String description) {
        super(name, ESensStereotype.DATE_TIME_DATA, description);
    }

    @Override
    public void read(String sentence) {

        Map map = SensNewParser.parseParameters(Arrays.asList("D:", "T:", "U:"), sentence);

        data = map.toString();
    }

    @Override
    public View getView(LayoutInflater inflater, View convertView, ViewGroup parent, SensClient client) {
        if (view == null || convertView == null) {
            view = convertView;
            if (convertView == null) {
                view = inflater.inflate(R.layout.sensadapter_s_item_layout, parent, false);
                toBind = true;
            }
        }

        if (toBind) {
            toBind = false;
            typeText = (TextView) view.findViewById(R.id.textRow1);
            nameText = (TextView) view.findViewById(R.id.textRow21);
            descriptionText = (TextView) view.findViewById(R.id.textRow22);
            textData = (TextView) view.findViewById(R.id.textData);
        }

        typeText.setText(type.getDescription());
        nameText.setText(name);
        descriptionText.setText(description);
        textData.setText(data);

        return view;
    }

}
