#include <Sensoria.h>

// Arduino "Secret" Temperature Sensor
<<<<<<< HEAD
//~ #include <SensoriaSensors/Temperature328.h>
//~ TemperatureSensor328 secretSensor;

#include <SensoriaSensors/HumiditySI7021.h>
SI7021HumiditySensor si7021;

#include <SensoriaCommunicators/Serial.h>
SensoriaSerialCommunicator comm;

#include <SensoriaInternals/Server.h>
SensoriaServer srv;;

void mypanic (int interval) {
=======
#include <SensoriaSensors/Temperature328.h>
TemperatureSensor328 secretSensor;

#include <SensoriaServers/Serial.h>
SensoriaSerialServer srv;

void panic (int interval) {
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
  pinMode (LED_BUILTIN, OUTPUT);
  while (42) {
    digitalWrite (LED_BUILTIN, HIGH);
    delay (interval);
    digitalWrite (LED_BUILTIN, LOW);
    delay (interval);
  }
}

void setup (void) {
<<<<<<< HEAD
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
=======
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
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
    }
  } else {
    DPRINTLN ("Sensor failed begin()");
  }

  // We're ready!
  Serial.println (F("GO!"));
}

void loop (void) {
<<<<<<< HEAD
  srv.loop ();
=======
  srv.receive ();
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
}
