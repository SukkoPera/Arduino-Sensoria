#include <Sensoria.h>

// Arduino "Secret" Temperature Sensor
//~ #include <SensoriaSensors/Temperature328.h>
//~ TemperatureSensor328 secretSensor;

#include <SensoriaSensors/HumiditySI7021.h>
SI7021HumiditySensor si7021;

#include <SensoriaCommunicators/Serial.h>
SensoriaSerialCommunicator comm;

#include <SensoriaCore/Server.h>
SensoriaServer srv;;

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
  Serial.begin (9600);
  comm.begin (Serial);
  if (!srv.begin (F("Indoor-1"), comm)) {
    mypanic (500);
  }

  //~ if (secretSensor.begin (F("HT"), F("Indoor Temp"))) {
  if (si7021.begin (F("HT"), F("Indoor Humid+Temp"))) {
    if (srv.addTransducer (si7021) >= 0) {
      DPRINT (F("Sensor registered: "));
      DPRINTLN (si7021.name);
    } else {
      mypanic (1000);
    }
  } else {
    DPRINTLN ("Sensor failed begin()");
  }

  // We're ready!
  Serial.println (F("GO!"));
}

void loop (void) {
  srv.loop ();
}
