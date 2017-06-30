package com.sensoria.typhosoft.sensapp.custom.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Switch;
import android.widget.TextView;

import com.sensoria.typhosoft.sensapp.R;
import com.sensoria.typhosoft.sensapp.data.SensStereotypeEnum;
import com.sensoria.typhosoft.sensapp.data.Transducer;
import com.sensoria.typhosoft.sensapp.data.Actuator;
import com.sensoria.typhosoft.sensapp.data.Sensor;
import com.sensoria.typhosoft.sensapp.data.SensorTypeEnum;

import java.util.List;

/**
 * Created by santonocitom on 27/06/17.
 */

public class SensAdapter extends ArrayAdapter<Transducer> {

    public LayoutInflater inflater;

    public SensAdapter(Context context, int resource, List<Transducer> items) {
        super(context, resource, items);
        setNotifyOnChange(true);
        inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public int getItemViewType(int position) {
        return getItem(position).getStereoType().ordinal();
    }


    @Override
    public int getViewTypeCount() {
        return SensStereotypeEnum.values().length;
    }


    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        Transducer currentRowItem = getItem(position);
        int itemViewType = getItemViewType(position);

        View vi = convertView;
        if (convertView == null) {
            switch (currentRowItem.getType()) {
                case ACTUATOR:
                    switch(currentRowItem.getStereoType()){
                        case WEATHER_DATA:
                            break;
                        case RELAY_DATA:
                            vi = inflater.inflate(R.layout.sensadapter_a_rs_item_layout, parent, false);
                            break;
                        case CONTROLLED_RELAY_DATA:
                            vi = inflater.inflate(R.layout.sensadapter_a_cr_item_layout, parent, false);
                            break;
                        case MOTION_DATA:
                            break;
                    }
                    break;
                case SENSOR:
                    vi = inflater.inflate(R.layout.sensadapter_s_item_layout, parent, false);
                    break;
                default:
                    vi = inflater.inflate(R.layout.sensadapter_s_item_layout, parent, false);

            }

        }

        // common
        TextView typeText = (TextView) vi.findViewById(R.id.textRow1);
        TextView nameText = (TextView) vi.findViewById(R.id.textRow21);
        TextView descriptionText = (TextView) vi.findViewById(R.id.textRow22);


        switch (currentRowItem.getType()) {
            case ACTUATOR:
                Actuator act = (Actuator) currentRowItem;
                Switch onOffSwitch = (Switch) vi.findViewById(R.id.switch1);
                onOffSwitch.setChecked(act.getOnOff());

                switch(currentRowItem.getStereoType()){
                    case WEATHER_DATA:
                        break;
                    case RELAY_DATA:
                        break;
                    case CONTROLLED_RELAY_DATA:
                        Switch autoManualSwitch = (Switch) vi.findViewById(R.id.switch2);
                        autoManualSwitch.setChecked(act.getAutoManual());
                        break;
                    case MOTION_DATA:
                        break;
                }
                break;
            case SENSOR:
                Sensor sens = (Sensor) currentRowItem;
                TextView dataText = (TextView) vi.findViewById(R.id.textData);
                dataText.setText(sens.getData());
                break;
            default:

        }


        String type = currentRowItem.getType().getDescription();
        String name = currentRowItem.getName();
        String description = currentRowItem.getDescriptor();
        typeText.setText(type);
        nameText.setText(name);
        descriptionText.setText(description);

        return vi;
    }
}
