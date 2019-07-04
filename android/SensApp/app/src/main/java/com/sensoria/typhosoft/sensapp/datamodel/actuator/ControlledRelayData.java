package com.sensoria.typhosoft.sensapp.datamodel.actuator;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import com.sensoria.typhosoft.sensapp.R;
import com.sensoria.typhosoft.sensapp.core.SensNewParser;
import com.sensoria.typhosoft.sensapp.datamodel.ESensCommand;
import com.sensoria.typhosoft.sensapp.datamodel.ESensStereotype;
import com.sensoria.typhosoft.sensapp.network.SensClient;
import com.sensoria.typhosoft.sensapp.network.SesSocketSingleton;

import java.util.Arrays;
import java.util.Map;

/**
 * Created by santonocitom on 15/01/18.
 */

public class ControlledRelayData extends Actuator {

    private EStatus status = EStatus.UNKNOWN;
    private EAuto auto = EAuto.UNKNOWN;
    private TextView typeText;
    private TextView nameText;
    private TextView descriptionText;
    private Switch onOffSwitch;
    private Switch autoManualSwitch;
    private boolean toBind = true;
    private View view;
    private OnOffSwitchListener onOffSwitchListener;
    private AutoManualSwitchListener autoManualSwitchListener;

    public ControlledRelayData(String name, String description) {
        super(name, ESensStereotype.CONTROLLED_RELAY_DATA, description);
    }

    @Override
    public void read(String sentence) {

        Map map = SensNewParser.parseParameters(Arrays.asList("S:", "C:"), sentence);

        if (map.get("S:").equals(RelayData.EStatus.ON.toString())) {
            status = EStatus.ON;
        } else if (map.get("S:").equals(RelayData.EStatus.OFF.toString())) {
            status = EStatus.OFF;
        } else {
            status = EStatus.UNKNOWN;
        }

        if (map.get("C:").equals(EAuto.AUT.toString())) {
            auto = EAuto.AUT;
        } else if (map.get("C:").equals(EAuto.MAN.toString())) {
            auto = EAuto.MAN;
        } else {
            auto = EAuto.UNKNOWN;
        }
    }

    @Override
    public View getView(LayoutInflater inflater, View convertView, ViewGroup parent) {
        if (view == null || convertView == null) {
            view = convertView;
            if (convertView == null) {
                view = inflater.inflate(R.layout.sensadapter_a_cr_item_layout, parent, false);
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
            onOffSwitchListener = new OnOffSwitchListener();
            autoManualSwitchListener = new AutoManualSwitchListener();
        }

        typeText.setText(type.getDescription());
        nameText.setText(name);
        descriptionText.setText(description);
        onOffSwitch.setOnCheckedChangeListener(null);
        autoManualSwitch.setOnCheckedChangeListener(null);
        switch (status) {
            case UNKNOWN:
                onOffSwitch.setEnabled(false);
                onOffSwitch.setChecked(false);
                onOffSwitch.setTextOff("UNKNOWN");
                break;
            case ON:
                onOffSwitch.setEnabled(true);
                onOffSwitch.setChecked(true);
                onOffSwitch.setTextOn("ON");
                break;
            case OFF:
                onOffSwitch.setEnabled(true);
                onOffSwitch.setChecked(false);
                onOffSwitch.setTextOff("OFF");
                break;
        }

        switch (auto) {
            case UNKNOWN:
                onOffSwitch.setEnabled(false);
                autoManualSwitch.setEnabled(false);
                autoManualSwitch.setChecked(false);
                autoManualSwitch.setTextOff("UNKNOWN");
                break;
            case AUT:
                onOffSwitch.setEnabled(false);
                autoManualSwitch.setEnabled(true);
                autoManualSwitch.setChecked(false);
                autoManualSwitch.setTextOn("AUT");
                break;
            case MAN:
                onOffSwitch.setEnabled(true);
                autoManualSwitch.setEnabled(true);
                autoManualSwitch.setChecked(true);
                autoManualSwitch.setTextOn("MAN");
                break;
        }
        onOffSwitch.setOnCheckedChangeListener(onOffSwitchListener);
        autoManualSwitch.setOnCheckedChangeListener(autoManualSwitchListener);
        return view;
    }

    @Override
    public String write() {
        return ESensCommand.WRI.getCmd().concat(" ").concat(name).concat(" S:").concat(status.name()).concat(" C:").concat(auto.name());
    }


    public EStatus getStatus() {
        return status;
    }

    public void setStatus(EStatus status) {
        this.status = status;
    }

    public EAuto getAuto() {
        return auto;
    }

    public void setAuto(EAuto auto) {
        this.auto = auto;
    }

    public enum EStatus {
        UNKNOWN,
        ON,
        OFF
    }

    public enum EAuto {
        UNKNOWN,
        AUT,
        MAN
    }


    private class AutoManualSwitchListener implements CompoundButton.OnCheckedChangeListener {

        public AutoManualSwitchListener() {
        }

        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            switch (auto) {
                case UNKNOWN:
                    break;
                case AUT:
                    if (isChecked) {
                        auto = EAuto.MAN;
                        SesSocketSingleton.getInstance().sendMessage(write());
                        onOffSwitch.setEnabled(true);
                    }
                    break;
                case MAN:
                    if (!isChecked) {
                        auto = EAuto.AUT;
                        SesSocketSingleton.getInstance().sendMessage(write());
                        onOffSwitch.setEnabled(false);
                    }
                    break;
            }
        }
    }

    private class OnOffSwitchListener implements CompoundButton.OnCheckedChangeListener {

        public OnOffSwitchListener() {

        }

        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            switch (status) {
                case UNKNOWN:
                    break;
                case ON:
                    if (!isChecked && auto.equals(EAuto.MAN)) {
                        status = EStatus.OFF;
                        SesSocketSingleton.getInstance().sendMessage(write());
                    }
                    break;
                case OFF:
                    if (isChecked && auto.equals(EAuto.MAN)) {
                        status = EStatus.ON;
                        SesSocketSingleton.getInstance().sendMessage(write());
                    }
                    break;
            }
        }
    }
}
