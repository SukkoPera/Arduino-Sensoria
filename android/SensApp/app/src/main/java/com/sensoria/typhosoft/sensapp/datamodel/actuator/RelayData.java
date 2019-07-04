package com.sensoria.typhosoft.sensapp.datamodel.actuator;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import com.sensoria.typhosoft.sensapp.R;
import com.sensoria.typhosoft.sensapp.datamodel.ESensCommand;
import com.sensoria.typhosoft.sensapp.datamodel.ESensStereotype;
import com.sensoria.typhosoft.sensapp.network.SensClient;

/**
 * Created by santonocitom on 15/01/18.
 */

public class RelayData extends Actuator {

    public EStatus status = EStatus.UNKNOWN;
    private TextView typeText;
    private TextView nameText;
    private TextView descriptionText;
    private Switch onOffSwitch;
    private Switch autoManualSwitch;
    private boolean toBind = true;
    private View view;
    private OnOffSwitchListener onOffSwitchListener;

    public RelayData(String name, String description) {
        super(name, ESensStereotype.RELAY_DATA, description);
    }

    @Override
    public void read(String sentence) {
        int index = sentence.lastIndexOf(" ") + 1;
        String lStatus = sentence.substring(index);
        if (lStatus.equals(EStatus.ON.toString())) {
            status = EStatus.ON;
        } else if (lStatus.equals(EStatus.OFF.toString())) {
            status = EStatus.OFF;
        } else {
            status = EStatus.UNKNOWN;
        }
    }

    @Override
    public View getView(LayoutInflater inflater, View convertView, ViewGroup parent, final SensClient client) {
        if (view == null || convertView == null) {
            view = convertView;
            if (convertView == null) {
                view =  inflater.inflate(R.layout.sensadapter_a_cr_item_layout, parent, false);
            toBind = true;
        }
    }

        if (toBind) {
        toBind = false;
            typeText = (TextView) view.findViewById(R.id.textRow1);
            nameText = (TextView) view.findViewById(R.id.textRow21);
            descriptionText = (TextView) view.findViewById(R.id.textRow22);
            onOffSwitch = (Switch) view.findViewById(R.id.switch1);
            autoManualSwitch = (Switch) view.findViewById(R.id.switch2);
            onOffSwitchListener = new OnOffSwitchListener(client);
        }

        typeText.setText(type.getDescription());
        nameText.setText(name);
        descriptionText.setText(description);
        autoManualSwitch.setVisibility(View.INVISIBLE);
        onOffSwitch.setOnCheckedChangeListener(null);
        switch (status) {
            case UNKNOWN:
                onOffSwitch.setEnabled(false);
                onOffSwitch.setChecked(false);
                onOffSwitch.setTextOff("UNKNOWN");
                break;
            case ON:
                onOffSwitch.setChecked(true);
                onOffSwitch.setTextOn("ON");
                break;
            case OFF:
                onOffSwitch.setChecked(false);
                onOffSwitch.setTextOff("OFF");
                break;
        }

        onOffSwitch.setOnCheckedChangeListener(onOffSwitchListener);
        return view;
    }

    @Override
    public String write() {
        return ESensCommand.WRI.getCmd().concat(" ").concat(name).concat(" ").concat(status.name());
    }

    public EStatus getStatus() {
        return status;
    }

    public void setStatus(EStatus status) {
        this.status = status;
    }

    public enum EStatus {
        UNKNOWN,
        ON,
        OFF;
    }

    private class OnOffSwitchListener implements CompoundButton.OnCheckedChangeListener {
        private final SensClient client;

        public OnOffSwitchListener(SensClient client) {
            this.client = client;
        }

        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            switch (status) {
                case UNKNOWN:
                    break;
                case ON:
                    if (!isChecked) {
                        status = EStatus.OFF;
                        client.sendMessage(write());
                    }
                    break;
                case OFF:
                    if (isChecked) {
                        status = EStatus.ON;
                        client.sendMessage(write());
                    }
                    break;
            }
        }
    }
}
