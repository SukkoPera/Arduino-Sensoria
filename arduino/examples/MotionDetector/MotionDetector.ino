#include <Sensoria.h>

#include <SensoriaSensors/MotionPIR.h>
MotionPIR motionDetector;

#include <SensoriaCommunicators/ESPStandAlone.h>
ESPCommunicator comm;

#include <SensoriaCore/Server.h>
SensoriaServer srv;

// Wi-Fi parameters
#define WIFI_SSID        "SukkoNet-TO"
#define WIFI_PASSWORD    "everythingyouknowiswrong"

const byte PIR_PIN = D8;

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
  DSTART (9600);

  if (!comm.begin (WIFI_SSID, WIFI_PASSWORD)) {
    mypanic (100);
  }

  if (!srv.begin (F("MotionDetector-1"), comm)) {
    mypanic (500);
  }

  if (motionDetector.begin (F("IM"), F("Indoor Motion"), PIR_PIN)) {
    if (srv.addTransducer (motionDetector) >= 0) {
      DPRINT (F("Sensor registered: "));
      DPRINTLN (motionDetector.name);
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

  // Also indicate motion with led (which is active low on WeMos stuff)
  digitalWrite (LED_BUILTIN, !digitalRead (PIR_PIN));
}
