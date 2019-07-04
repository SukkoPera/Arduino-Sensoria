package com.sensoria.typhosoft.sensapp.datamodel;

import android.support.annotation.NonNull;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.sensoria.typhosoft.sensapp.R;
import com.sensoria.typhosoft.sensapp.core.ISensAdapterItems;
import com.sensoria.typhosoft.sensapp.network.SensClient;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by santonocitom on 29/06/17.
 */

public class Node implements ISensAdapterItems {

    protected List<ATransducer> transducers = new ArrayList<>();
    private String name;
    private TextView nameText;
    private boolean toInit = true;

    public Node(String nodeName) {
        name = nodeName;
    }

    public Node() {
        this("Server X");
    }

    public List<ATransducer> getTransducers() {
        return transducers;
    }

    public void setTransducers(List<ATransducer> transducers) {
        this.transducers = transducers;
    }

    public ESensType getType() {
        return ESensType.NODE;
    }

    public String getName() {
        return name;
    }

    @Override
    public View getView(LayoutInflater inflater, View convertView, ViewGroup parent) {
        View vi = convertView;
        if (vi == null) {
            vi = inflater.inflate(R.layout.sensadapter_node_item_layout, parent, false);
        }
        if(toInit){
            nameText = (TextView) vi.findViewById(R.id.textRow21);
            toInit = true;
        }

        nameText.setText(name);
        return vi;
    }

    @Override
    public ESensStereotype getStereoType() {
        return ESensStereotype.NODE;
    }

    @Override
    public int hashCode() {
        return this.name.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof Node)
            return this.name.equalsIgnoreCase(((Node) obj).name);

        return false;
    }

    @Override
    public int compareTo(@NonNull Object t) {
        if (t instanceof ATransducer)
            return this.name.compareTo(((ATransducer) t).getName());
        else if (t instanceof Node)
            return this.name.compareTo(((Node) t).getName());

        return 0;
    }
}