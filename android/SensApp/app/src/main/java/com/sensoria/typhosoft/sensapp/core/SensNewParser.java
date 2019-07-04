package com.sensoria.typhosoft.sensapp.core;

import com.sensoria.typhosoft.sensapp.datamodel.ATransducer;
import com.sensoria.typhosoft.sensapp.datamodel.ESensCommand;
import com.sensoria.typhosoft.sensapp.datamodel.ESensStereotype;
import com.sensoria.typhosoft.sensapp.datamodel.Node;
import com.sensoria.typhosoft.sensapp.datamodel.actuator.ControlledRelayData;
import com.sensoria.typhosoft.sensapp.datamodel.actuator.RelayData;
import com.sensoria.typhosoft.sensapp.datamodel.actuator.TimeControlData;
import com.sensoria.typhosoft.sensapp.datamodel.sensor.DateTimeData;
import com.sensoria.typhosoft.sensapp.datamodel.sensor.UnknownData;
import com.sensoria.typhosoft.sensapp.datamodel.sensor.MotionData;
import com.sensoria.typhosoft.sensapp.datamodel.sensor.WeatherData;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by santonocitom on 12/01/18.
 */

public class SensNewParser {

    /*
    Command:
  <command> [args]

    Invalid command reply:
    ERR [reason]
    This reply MUST be sent when the server does not understand the command at all, for instance when an unsupported command is sent. If reason is not specified, it is assumed to be Unsupported command.

    Successful command reply:
  <command> OK [args]
    This reply MUST be sent when the server has evaluated a command correctly.

    Failure command reply:
  <command> ERR [reason]
    This reply MUST be sent when the server has correctly recognized a command but it has encountered an error during the evaluation of the command, which overall failed. A detailed failure reason can be provided in the reply (e.g.: Invalid parameters, Sensor error, etc...).

*/


    public static ESensCommand parseCommand(String receivedString) {
        System.out.println(receivedString);
        int firstSpace = receivedString.indexOf(" ");
        String cmd;
        if(firstSpace > 0)
            cmd = receivedString.substring(0, firstSpace);
        else
            return null;
        System.out.println("Command: " + cmd);
        return ESensCommand.convert(cmd);
    }

    /*
      Client -> Server
        HLO: Request node information
        Request: HLO
        Reply: HLO <node_name> (<name> <type> <stereotype>)|...
    Example:
      <-- HLO
      --> HLO Outdoor-1 OT S WD|OH S WD|OP S WD
    */
    public static Node parseHLO(String reply) {
        reply = reply.substring(reply.indexOf(" ") + 1);

        System.out.println("parseHLO(" + reply + ")");
        String nodeName = reply.substring(0, reply.indexOf(" "));
        Node node = new Node(nodeName);
        System.out.println("nodeName: " + nodeName);
        reply = reply.substring(reply.indexOf(" ") + 1);
        String[] strings = reply.split("\\|");
        for (String transducer :
                strings) {
            System.out.println("Transducer: " + transducer);
            String[] split = transducer.split(" ");
            if (split.length >= 3) {
                String name = split[0];
                String type = split[1];
                String stereotype = split[2];

                String description = new String();
                if (split.length >= 4) {
                    for (int idx = 3; idx < split.length; idx++) {
                        description = description.concat(" ").concat(split[idx]);
                    }
                }
                ESensStereotype st = ESensStereotype.convert(stereotype);
                ATransducer aTransducer;
                if(st == null){
                    st = ESensStereotype.UNKNOWN;
                }
                switch (st) {
                    case WEATHER_DATA:
                        aTransducer = new WeatherData(name, description);
                        break;
                    case RELAY_DATA:
                        aTransducer = new RelayData(name, description);
                        break;
                    case CONTROLLED_RELAY_DATA:
                        aTransducer = new ControlledRelayData(name, description);
                        break;
                    case MOTION_DATA:
                        aTransducer = new MotionData(name, description);
                        break;
                    case TIME_CONTROL_DATA:
                        aTransducer = new TimeControlData(name, description);
                        break;
                    case DATE_TIME_DATA:
                        aTransducer = new DateTimeData(name, description);
                        break;
                    case VALUE_SET_DATA:
                    case UNKNOWN:
                    case NODE:
                    default:
                        aTransducer = new UnknownData(name, description);
                }

                if (aTransducer != null) {
                    node.getTransducers().add(aTransducer);
                }
            }
        }
        return node;
    }


    public static String parseTransducerName(String reply) {
        String name = null;
        System.out.println("parseTransducerName: " + reply);
        String[] strings = reply.split(" ");
        if (strings.length > 2)
            name = strings[1];
        System.out.println("name: " + name);
        return name;
    }

    /*
        REA: Read a transducer.
        Request: REA <transducer_name>
        Reply: REA OK <data>
    Examples:
      <-- REA OH
      --> REA OK T:11.00 H:40.00
      <-- REA TT
      --> REA ERR No such transducer: TT
    Data reported in the reply MUST be formatted according to the stereotype the transducer conforms to, which is reported in the reply to the HLO/QRY commands.
        WRI: Write to an actuator. Sensors cannot be written to.
        Request: WRI <actuator_name> <value>
    Examples:
      <-- WRI RH AUT
      --> WRI OK
      <-- WRI RB MAN
      --> WRI ERR No such actuator: RB
    Data provided in the reply MUST be formatted according to the stereotype the actuator conforms to, which is reported in the reply to the HLO/QRY commands.
    */
    private static void parseREAWRI(String reply) {
        System.out.println("parseREA(" + reply + ")");
        String retVal = reply.substring(0, reply.indexOf(" "));
        String data = reply.substring(reply.indexOf(" ") + 1);
        System.out.println("retVal: " + retVal);
        System.out.println("data: " + data);
    }
    /*
    CFG: Configure a setting on a transducer.
    Not yet implemented

    NRQ: Issue a Notification Request.
    Request: NRQ <transducer_name> <notification_type> [args]
notification_type can either be PRD for periodic notifications or CHA in order to be notified when the specified transducer changes value. In the former case, args MUST be an integer, representing the desired interval in seconds between notifications.
Examples:
  <-- NRQ OT CHA
  --> NRQ OK
  <-- NRQ OT PRD 60
  --> NRQ OK
    NDL: Delete a Notification Request.
    Request: NDL <transducer_name> <notification_type>
Examples:
  <-- NDL OT CHA
  --> NDL OK

    NCL: Clear all notification requests.
    Request: NCL
Examples:
  <-- NCL
  --> NCL OK

Server -> Client
    NOT: Notification.
    HLO: Server advertisement.
Not yet implemented
  */

    public static String parseParameter(String key, String reply) {
        String value = null;
        if (reply.contains(key)) {
            int keyIndex = reply.indexOf(key) + key.length();
            int valueIndex = reply.indexOf(" ", keyIndex);
            if (valueIndex < keyIndex)
                valueIndex = reply.length();
            value = reply.substring(keyIndex, valueIndex);
        }
        return value;
    }

    public static Map parseParameters(List<String> keys, String reply) {
        Map values = new HashMap<String, String>();
        for (String key:keys){
            values.put(key, parseParameter(key, reply));
        }
        return values;
    }
}
