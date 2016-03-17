#include <Sensoria.h>

#include <SensoriaActuators/Relay.h>
Relay relay;

#include <SensoriaCommunicators/Serial.h>
SensoriaSerialCommunicator comm;

#include <SensoriaCore/Server.h>
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
  Serial.begin (9600);
  comm.begin (Serial);
  if (!srv.begin (F("Actuator-1"), comm)) {
    mypanic (500);
  }

  if (relay.begin (F("RR"), F("Demo Relay"), 13)) {
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
