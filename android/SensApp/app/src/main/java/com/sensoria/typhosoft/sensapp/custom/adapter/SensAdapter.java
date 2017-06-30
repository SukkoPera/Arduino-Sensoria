package com.sensoria.typhosoft.sensapp.custom.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import com.sensoria.typhosoft.sensapp.R;
import com.sensoria.typhosoft.sensapp.data.Actuator;
import com.sensoria.typhosoft.sensapp.data.SensCommandEnum;
import com.sensoria.typhosoft.sensapp.data.SensStereotypeEnum;
import com.sensoria.typhosoft.sensapp.data.Sensor;
import com.sensoria.typhosoft.sensapp.data.Transducer;
import com.sensoria.typhosoft.sensapp.network.SensClient;

import java.util.List;

/**
 * Created by santonocitom on 27/06/17.
 */

public class SensAdapter extends ArrayAdapter<Transducer> {

    public final LayoutInflater inflater;
    private final SensClient sensClient;

    public SensAdapter(Context context, int resource, List<Transducer> items, SensClient sensClient) {
        super(context, resource, items);
        this.sensClient = sensClient;
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
        View vi = convertView;
        if (convertView == null) {
            switch (currentRowItem.getType()) {
                case ACTUATOR:
                    switch (currentRowItem.getStereoType()) {
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


            ViewHolder holder = new ViewHolder();
            holder.typeText = (TextView) vi.findViewById(R.id.textRow1);
            holder.nameText = (TextView) vi.findViewById(R.id.textRow21);
            holder.descriptionText = (TextView) vi.findViewById(R.id.textRow22);
            holder.dataText = (TextView) vi.findViewById(R.id.textData);
            holder.onOffSwitch = (Switch) vi.findViewById(R.id.switch1);
            holder.autoManualSwitch = (Switch) vi.findViewById(R.id.switch2);

            if(holder.onOffSwitch != null){
                holder.onOffSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                    @Override
                    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                        ViewHolder holder = (ViewHolder) ((View)buttonView.getParent()).getTag();
                        String name = holder.nameText.getText().toString();
                        sensClient.sendMessage(SensCommandEnum.WRI.getCmd() + " " + name + " " + (isChecked ? "ON":"OFF"));
                    }
                });
            }

            if(holder.autoManualSwitch != null){


            }

            vi.setTag(holder);
        }

        ViewHolder holder = (ViewHolder) vi.getTag();

        switch (currentRowItem.getType()) {
            case ACTUATOR:
                Actuator act = (Actuator) currentRowItem;
                holder.onOffSwitch.setChecked(act.getOnOff());

                switch (currentRowItem.getStereoType()) {
                    case WEATHER_DATA:
                        break;
                    case RELAY_DATA:
                        break;
                    case CONTROLLED_RELAY_DATA:
                        holder.autoManualSwitch.setChecked(act.getAutoManual());
                        break;
                    case MOTION_DATA:
                        break;
                }
                break;
            case SENSOR:
                Sensor sens = (Sensor) currentRowItem;
                holder.dataText.setText(sens.getData());
                break;
            default:

        }


        String type = currentRowItem.getType().getDescription();
        String name = currentRowItem.getName();
        String description = currentRowItem.getDescriptor();
        holder.typeText.setText(type);
        holder.nameText.setText(name);
        holder.descriptionText.setText(description);

        return vi;
    }

    static class ViewHolder {
        TextView typeText;
        TextView nameText;
        TextView descriptionText;
        TextView dataText;
        Switch onOffSwitch;
        Switch autoManualSwitch;
    }
}
