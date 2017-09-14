#include <Sensoria.h>
#include <SensoriaSensors/HumiditySHT30.h>

SHT30HumiditySensor sensor;

#include <SensoriaCommunicators/ESPStandAlone.h>
ESPCommunicator comm;

#include <SensoriaCore/Server.h>
SensoriaServer srv;

// Wi-Fi parameters
#define WIFI_SSID        "essid"
#define WIFI_PASSWORD    "password"


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

  if (!comm.begin (WIFI_SSID, WIFI_PASSWORD)) {
    mypanic (100);
  }

  if (!srv.begin (F("ESPSHT30"), comm)) {
    mypanic (500);
  }

  if (sensor.begin (F("IT"), F("Indoor Temperature"))) {
    if (srv.addTransducer (sensor) >= 0) {
      DPRINT (F("Sensor registered: "));
      DPRINTLN (sensor.name);
    } else {
      mypanic (1000);
    }
  } else {
    DPRINTLN ("Sensor failed begin()");
  }

  // Signal we're ready!
  //Serial.println (F("GO!"));
  pinMode (LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite (LED_BUILTIN, LOW);
    delay (100);
    digitalWrite (LED_BUILTIN, HIGH);
    delay (100);
  }
}

void loop (void) {
  srv.loop ();
}
