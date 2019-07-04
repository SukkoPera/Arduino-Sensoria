package com.sensoria.typhosoft.sensapp.core;

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
    private SensClient client;

    public SensAdapter(SensActivity context, int resource, SensClient sensClient) {
        super(context, resource, SensController.getInstance().adapterItems);
        this.client = sensClient;
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
        //TODO: remove porcata sensClient
        return currentRowItem.getView(inflater, convertView,parent, client);
    }

    public void setClient(SensClient client) {
        this.client = client;
    }
}
