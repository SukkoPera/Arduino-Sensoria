#include <SoftwareSerial.h>
#include <Sensoria.h>
#include <SensoriaCore/Server.h>
#include <SensoriaCommunicators/ESPWifiAlt.h>

#include <SensoriaActuators/Relay.h>

Relay relay;

// Wi-Fi parameters
#define WIFI_SSID        "ssid"
#define WIFI_PASSWORD    "password"

SoftwareSerial swSerial (10, 11);

SensoriaEsp8266Communicator comm;
SensoriaServer srv;

void mypanic (int interval) {
  pinMode (LED_BUILTIN, OUTPUT);
  while (42) {
    digitalWrite (LED_BUILTIN, HIGH);
    delay (interval);
    digitalWrite (LED_BUILTIN, LOW);
    delay (interval);
  }
}

void setup (void) {
  DSTART ();

  swSerial.begin (9600);
  if (!comm.begin (swSerial, WIFI_SSID, WIFI_PASSWORD)) {
    mypanic (100);
  }

  if (!srv.begin (F("RelayTest"), comm)) {
    mypanic (500);
  }

  if (relay.begin (F("RR"), F("Demo Relay"), LED_BUILTIN)) {
    if (srv.addTransducer (relay) >= 0) {
      DPRINT (F("Actuator registered: "));
      DPRINTLN (relay.name);
    } else {
      mypanic (1000);
    }
  } else {
    DPRINTLN (F("Actuator failed begin()"));
  }

  // We're ready!
  Serial.println (F("GO!"));
}

void loop (void) {
  srv.loop ();
}
