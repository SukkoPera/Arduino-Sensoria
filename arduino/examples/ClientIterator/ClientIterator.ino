#include <Sensoria.h>

#include <SensoriaClient/SensoriaClient.h>
SensoriaClient client;

#include <SensoriaCommunicators/ESPStandAlone.h>
ESPCommunicator comm;

// Wi-Fi parameters
#define WIFI_SSID        "ssid"
#define WIFI_PASSWORD    "password"


#define LED_PIN LED_BUILTIN

void mypanic (int interval) {
  pinMode (LED_PIN, OUTPUT);
  while (42) {
    digitalWrite (LED_PIN, HIGH);
    delay (interval);
    digitalWrite (LED_PIN, LOW);
    delay (interval);
  }
}

void setup() {
  Serial.begin (9600);

  if (!comm.begin (WIFI_SSID, WIFI_PASSWORD)) {
    mypanic (100);
  }

  client.begin (comm);
}

void loop() {
  Serial.println (F("-- NODES --"));
  SensoriaIterator iter = client.getIterator ();
  ServerProxy *s;
  while ((s = iter.nextNode ())) {
    Serial.println (s -> name);
  }
  Serial.println (F("-----------"));

  Serial.println (F("-- TRANSDUCERS --"));
  iter = client.getIterator ();
  TransducerProxy *tpx;
  while ((tpx = iter.nextTransducer ())) {
    Serial.println (tpx -> name);
  }
  Serial.println (F("-----------------"));

  Serial.println (F("-- SENSORS --"));
  iter = client.getIterator ();
  SensorProxy *spx;
  while ((spx = iter.nextSensor ())) {
    Serial.print (spx -> name);
    Serial.print (F(": "));

    Stereotype *st;
    if (!(spx -> read (st))) {
      Serial.println (F("Read FAILED"));
    } else {
      Serial.println (F("Read OK"));
    }
  }
  Serial.println (F("-------------"));

  Serial.println (F("-- ACTUATORS --"));
  iter = client.getIterator ();
  ActuatorProxy *apx;
  while ((apx = iter.nextActuator ())) {
    Serial.println (apx -> name);
  }
  Serial.println (F("---------------"));

  client.loop ();

  delay (30000UL);
}
