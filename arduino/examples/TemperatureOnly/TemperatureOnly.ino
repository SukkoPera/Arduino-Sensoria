#include <SoftwareSerial.h>
#include <Sensoria.h>
#include <SensoriaInternals/Server.h>
#include <SensoriaCommunicators/ESPWifiAlt.h>

#include <SensoriaSensors/Temperature328.h>

TemperatureSensor328 tempSensor;

// Wi-Fi parameters
#define WIFI_SSID        "ssid"
#define WIFI_PASSWORD    "password"

#define LED_PIN LED_BUILTIN

SoftwareSerial swSerial (10, 11);

SensoriaEsp8266Communicator comm;
SensoriaServer srv;


void mypanic (int interval) {
  pinMode (LED_PIN, OUTPUT);
  while (42) {
    digitalWrite (LED_PIN, HIGH);
    delay (interval);
    digitalWrite (LED_PIN, LOW);
    delay (interval);
  }
}

void setup (void) {
  DSTART ();

  swSerial.begin (9600);
  if (!comm.begin (swSerial, WIFI_SSID, WIFI_PASSWORD)) {
    mypanic (100);
  }

  if (!srv.begin (F("TemperatureTest"), comm)) {
    mypanic (500);
  }

  if (tempSensor.begin (F("IT"), F("Indoor Temperature"))) {
    if (srv.addTransducer (tempSensor) >= 0) {
      DPRINT (F("Sensor registered: "));
      DPRINTLN (tempSensor.name);
    } else {
      mypanic (1000);
    }
  } else {
    DPRINTLN ("Sensor failed begin()");
  }

  // Signal we're ready!
  //Serial.println (F("GO!"));
  pinMode (LED_PIN, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite (LED_PIN, HIGH);
    delay (100);
    digitalWrite (LED_PIN, LOW);
    delay (100);
  }
}

void loop (void) {
  srv.loop ();
}
