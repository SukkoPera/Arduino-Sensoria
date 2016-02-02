#include <OneWire.h>
#include <DallasTemperature.h>
#include <DallasTemperatureSensor.h>

#include <Sensoria.h>
#include <SensoriaServers/ESPStandAlone.h>

OneWire oneWire (D2);
DallasTemperature sensors (&oneWire);
DallasTemperatureSensor dallasSensor;

// Wi-Fi parameters
#define SSID        "ssid"
#define PASSWORD    "password"""

// Pin 2 seems to control the ESP module led on my NodeMCU
#define LED_PIN BUILTIN_LED

ESPServer srv;


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

  if (!srv.begin (F("ESPDallas"), SSID, PASSWORD)) {
    mypanic (500);
  }

  // Init DS18B20
  // locate devices on the bus
  sensors.begin ();
  DPRINT (F("Found "));
  DPRINT (sensors.getDeviceCount (), DEC);
  DPRINTLN (F(" device(s)"));

  DeviceAddress outdoorThermometer;
  if (!sensors.getAddress (outdoorThermometer, 0)) {
    DPRINTLN (F("Unable to find address for Device 0"));
    mypanic (333);
  }

#if 0
  DPRINT (F("Device 0 Address: "));
  printAddress (insideThermometer);
  DPRINTLN ();
#endif

  // Set resolution to 9/10/11/12 bits (better precision = slower)
  sensors.setResolution (outdoorThermometer, 12);

  if (dallasSensor.begin (F("IT"), F("Indoor Temperature"), &sensors, outdoorThermometer)) {
    if (srv.addTransducer (dallasSensor) >= 0) {
      DPRINT (F("Sensor registered: "));
      DPRINTLN (dallasSensor.name);
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
  srv.receive ();
}
