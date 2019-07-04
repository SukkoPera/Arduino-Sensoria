package com.sensoria.typhosoft.sensapp.datamodel;

/**
 * Created by santonocitom on 29/06/17.
 */

public enum ESensCommand {
    VER("VER"),
    HLO("HLO"),
    REA("REA"),
    WRI("WRI"),
    QRY("QRY"),
    NRQ("NRQ"),
    ERR("ERR");

    private final String cmd;

    ESensCommand(String cmd) {
        this.cmd = cmd;
    }

    public final String getCmd() {
        return cmd;
    }

    public static ESensCommand convert(String cmd){
        for (ESensCommand c: ESensCommand.values()) {
            if(c.cmd.equalsIgnoreCase(cmd))
                return c;
        }
        return null;
    }
}
