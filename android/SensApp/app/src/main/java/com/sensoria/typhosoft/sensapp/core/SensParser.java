package com.sensoria.typhosoft.sensapp.core;

import com.sensoria.typhosoft.sensapp.data.Actuator;
import com.sensoria.typhosoft.sensapp.data.Node;
import com.sensoria.typhosoft.sensapp.data.SensCommandEnum;
import com.sensoria.typhosoft.sensapp.data.SensModel;
import com.sensoria.typhosoft.sensapp.data.Sensor;
import com.sensoria.typhosoft.sensapp.data.SensorTypeEnum;
import com.sensoria.typhosoft.sensapp.data.Transducer;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by santonocitom on 29/06/17.
 */

public class SensParser {

    public static SensCommandEnum parseCommand(String cmd) {

        return SensCommandEnum.valueOf(cmd.substring(0, 3));
    }

    public static Transducer makeSens(String item) {
        Transducer sensor = null;
        SensorTypeEnum sensorType = SensorTypeEnum.convert(item.substring(3, 4));
        switch (sensorType) {
            case ACTUATOR:
                sensor = new Actuator(item);
                break;
            case NODE:
                sensor = new Node(item);
                break;
            case SENSOR:
                sensor = new Sensor(item);
                break;
        }

        return sensor;
    }

    public static List<Transducer> makeSensors(String items) {
        ArrayList<Transducer> sensors = new ArrayList<>();
        String payload = items.substring(4);
        String[] itemsStrings = payload.split("\\|");
        for (String item : itemsStrings) {
            sensors.add(SensParser.makeSens(item));
        }

        return sensors;
    }

    public static String parseName(String sentence) {
        switch (parseCommand(sentence)) {
            case VER:
                break;
            case HLO:
                break;
            case REA:
                return sentence.substring(4, 6);
            case WRI:
                break;
            case QRY:
                break;
            case NRQ:
                break;
        }

        return null;
    }

    public static void parseREA(String sentence) {
        Transducer transducer = SensModel.getInstance().getItemsByName(SensParser.parseName(sentence));
        if (transducer != null)
            switch (transducer.getType()) {
                case ACTUATOR:
                    Actuator actuator = ((Actuator) transducer);
                    parseActuatorREA(sentence, actuator);
                    break;
                case SENSOR:
                    Sensor sensor = ((Sensor) transducer);
                    parseSensorREA(sentence, sensor);
                    break;
                case NODE:
                    break;
            }

    }

    private static void parseSensorREA(String sentence, Sensor sensor) {
        sensor.setData(sentence.substring(7));
    }

    private static void parseActuatorREA(String sentence, Actuator actuator) {
        switch (actuator.getStereoType()) {
            case WEATHER_DATA:
                break;
            case RELAY_DATA:
                actuator.setOnOff(sentence.substring(8).contains("ON"));
                break;
            case CONTROLLED_RELAY_DATA:
                actuator.setAutoManual(sentence.substring(10).contains("AUT"));
                break;
            case MOTION_DATA:
                break;
        }
    }
}
