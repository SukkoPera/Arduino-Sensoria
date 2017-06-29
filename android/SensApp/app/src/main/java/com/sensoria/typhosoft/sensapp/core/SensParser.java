package com.sensoria.typhosoft.sensapp.core;

import com.sensoria.typhosoft.sensapp.data.ASensor;
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

    public static ASensor makeSens(String item) {
        ASensor sensor = null;
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
            case TRANSDUCER:
                sensor = new Transducer(item);
                break;
        }

        return sensor;
    }

    public static List<ASensor> makeSensors(String items) {
        ArrayList<ASensor> sensors = new ArrayList<>();
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
        ASensor sensor = SensModel.getInstance().getItemsByName(SensParser.parseName(sentence));
        if (sensor != null)
            switch (sensor.getType()) {
                case ACTUATOR:
                    ((Actuator) sensor).setOnOff(parseActuatorREA(sentence));
                    break;
                case NODE:
                    break;
                case SENSOR:
                    ((Sensor) sensor).setData(parseSensorREA(sentence));
                    break;
                case TRANSDUCER:
                    break;
            }

    }

    private static String parseSensorREA(String sentence) {
        return sentence.substring(7);
    }

    private static Boolean parseActuatorREA(String sentence) {
        return sentence.substring(8).contains("ON");
    }
}
