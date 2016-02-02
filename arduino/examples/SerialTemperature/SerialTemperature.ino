#include <SensoriaServer.h>

// Arduino "Secret" Temperature Sensor
#include <TemperatureSensor328.h>
TemperatureSensor328 secretSensor;

#include <SerialServer.h>
SensoriaSerialServer srv;

void panic (int interval) {
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

  if (!srv.begin (F("Indoor-1"))) {
    panic (500);
  }

  if (secretSensor.begin (F("HT"), F("Indoor Temp"))) {
    if (srv.addTransducer (secretSensor) >= 0) {
      DPRINT (F("Sensor registered: "));
      DPRINTLN (secretSensor.name);
    } else {
      panic (1000);
    }
  } else {
    DPRINTLN ("Sensor failed begin()");
  }

  // We're ready!
  Serial.println (F("GO!"));
}

void loop (void) {
  srv.receive ();
}
