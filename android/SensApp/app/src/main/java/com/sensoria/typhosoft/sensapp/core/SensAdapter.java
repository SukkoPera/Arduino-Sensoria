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
import com.sensoria.typhosoft.sensapp.SensActivity;
import com.sensoria.typhosoft.sensapp.datamodel.actuator.Actuator;
import com.sensoria.typhosoft.sensapp.datamodel.ESensCommand;
import com.sensoria.typhosoft.sensapp.datamodel.ESensStereotype;
import com.sensoria.typhosoft.sensapp.datamodel.sensor.Sensor;
import com.sensoria.typhosoft.sensapp.network.SensClient;

/**
 * Created by santonocitom on 27/06/17.
 */

public class SensAdapter extends ArrayAdapter<ISensAdapterItems> {

    public final LayoutInflater inflater;
    private final SensClient sensClient;

    public SensAdapter(SensActivity context, int resource, SensClient sensClient) {
        super(context, resource);
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
        return ESensStereotype.values().length;
    }


    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ISensAdapterItems currentRowItem = getItem(position);
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
                case NODE:
                    vi = inflater.inflate(R.layout.sensadapter_node_item_layout, parent, false);
                    break;
                default:
                    // TODO must be substitute with error item
                    vi = inflater.inflate(R.layout.sensadapter_s_item_layout, parent, false);

            }


            ViewHolder holder = new ViewHolder(currentRowItem);
            holder.setTypeText((TextView) vi.findViewById(R.id.textRow1));
            holder.setNameText((TextView) vi.findViewById(R.id.textRow21));
            holder.setDescriptionText((TextView) vi.findViewById(R.id.textRow22));
            holder.setDataText((TextView) vi.findViewById(R.id.textData));
            holder.setOnOffSwitch((Switch) vi.findViewById(R.id.switch1));
            holder.setAutoManualSwitch((Switch) vi.findViewById(R.id.switch2));

            if (holder.onOffSwitch != null) {
                holder.onOffSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                    @Override
                    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                        ViewHolder holder = (ViewHolder) buttonView.getTag();
                        Actuator actuator = (Actuator) holder.getTransducer();
                        String name = actuator.getName();
                        if (isChecked != actuator.getOnOff()) {
                            if (holder.getAutoManualSwitch() != null) {
                                sensClient.sendMessage(ESensCommand.WRI.getCmd() + " " + name + " C:" + (isChecked ? "ON" : "OFF") + " S:" + (holder.getAutoManualSwitch().isChecked() ? "AUT" : "MAN"));
                            } else {
                                sensClient.sendMessage(ESensCommand.WRI.getCmd() + " " + name + " " + (isChecked ? "ON" : "OFF"));
                            }
                            actuator.setOnOff(isChecked);
                        }
                    }
                });
            }

            if (holder.autoManualSwitch != null) {
                holder.autoManualSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                    @Override
                    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                        ViewHolder holder = (ViewHolder) buttonView.getTag();
                        Actuator actuator = (Actuator) holder.getTransducer();
                        String name = actuator.getName();
                        if (isChecked != actuator.getAutoManual()) {
                            sensClient.sendMessage(ESensCommand.WRI.getCmd() + " " + name + " C:" + (holder.getOnOffSwitch().isChecked() ? "ON" : "OFF") + " S:" + (isChecked ? "AUT" : "MAN"));
                            actuator.setAutoManual(isChecked);
                        }
                    }
                });
            }

            vi.setTag(holder);
        }

        ViewHolder holder = (ViewHolder) vi.getTag();

        switch (currentRowItem.getType()) {
            case ACTUATOR:
                Actuator act = (Actuator) currentRowItem;
                ViewHolder.setSwitch(holder.onOffSwitch, act.getOnOff());

                switch (currentRowItem.getStereoType()) {
                    case WEATHER_DATA:
                        break;
                    case RELAY_DATA:
                        break;
                    case CONTROLLED_RELAY_DATA:
                        ViewHolder.setSwitch(holder.autoManualSwitch, act.getAutoManual());
                        break;
                    case MOTION_DATA:
                        break;
                }
                break;
            case SENSOR:
                Sensor sens = (Sensor) currentRowItem;
                ViewHolder.setText(holder.dataText, sens.getData());
                break;
            case NODE:

                break;
            default:

        }


        String type = currentRowItem.getType().getDescription();
        String name = currentRowItem.getName();
        String description = currentRowItem.getDescriptor();
        ViewHolder.setText(holder.typeText, type);
        ViewHolder.setText(holder.nameText, name);
        ViewHolder.setText(holder.descriptionText, description);

        return vi;
    }

    static class ViewHolder {
        private TextView typeText;
        private TextView nameText;
        private TextView descriptionText;
        private TextView dataText;
        private Switch onOffSwitch;
        private Switch autoManualSwitch;
        private ISensAdapterItems transducer;

        public ViewHolder(ISensAdapterItems transducer) {
            setTransducer(transducer);
        }

        public static void setText(TextView text, String value) {
            if (text != null) {
                text.setText(value);
            }
        }

        public static void setSwitch(Switch lSwitch, boolean value) {
            if (lSwitch != null) {
                lSwitch.setChecked(value);
            }
        }

        public static void setTag(View view, ViewHolder holder) {
            if (view != null) {
                view.setTag(holder);
            }
        }

        public ISensAdapterItems getTransducer() {
            return transducer;
        }

        public void setTransducer(ISensAdapterItems transducer) {
            this.transducer = transducer;
        }

        public TextView getTypeText() {
            return typeText;
        }

        public void setTypeText(TextView typeText) {
            this.typeText = typeText;
            setTag(typeText, this);
        }

        public TextView getNameText() {
            return nameText;
        }

        public void setNameText(TextView nameText) {
            this.nameText = nameText;
            setTag(nameText, this);
        }

        public TextView getDescriptionText() {
            return descriptionText;
        }

        public void setDescriptionText(TextView descriptionText) {
            this.descriptionText = descriptionText;
            setTag(descriptionText, this);
        }

        public TextView getDataText() {
            return dataText;
        }

        public void setDataText(TextView dataText) {
            this.dataText = dataText;
            setTag(dataText, this);
        }

        public Switch getOnOffSwitch() {
            return onOffSwitch;
        }

        public void setOnOffSwitch(Switch onOffSwitch) {
            this.onOffSwitch = onOffSwitch;
            setTag(onOffSwitch, this);
        }

        public Switch getAutoManualSwitch() {
            return autoManualSwitch;
        }

        public void setAutoManualSwitch(Switch autoManualSwitch) {
            this.autoManualSwitch = autoManualSwitch;
            setTag(autoManualSwitch, this);
        }
    }
}
