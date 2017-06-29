package com.sensoria.typhosoft.sensapp.data;

/**
 * Created by santonocitom on 29/06/17.
 */

public enum SensCommandEnum {
    VER("VER"),
    HLO("HLO"),
    REA("REA"),
    WRI("WRI"),
    QRY("QRY"),
    NRQ("NRQ");

    private final String cmd;

    SensCommandEnum(String cmd) {
        this.cmd = cmd;
    }

    public final String getCmd() {
        return cmd;
    }
}
