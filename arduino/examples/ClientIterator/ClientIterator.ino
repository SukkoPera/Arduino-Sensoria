#include <SoftwareSerial.h>
#include <Sensoria.h>
#include <SensoriaClient/SensoriaClient.h>
#include <SensoriaCommunicators/ESPWifiAlt.h>

IPAddress outdoor1(192, 168, 1, 170);
IPAddress outdoor2(192, 168, 1, 171);

// Wi-Fi parameters
#define WIFI_SSID        "ssid"
#define WIFI_PASSWORD    "password"

SoftwareSerial swSerial (10, 11);
SensoriaEsp8266Communicator comm;
SensoriaClient client;


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

  swSerial.begin (9600);
  if (!comm.begin (swSerial, WIFI_SSID, WIFI_PASSWORD)) {
    mypanic (100);
  }

  client.begin (comm);

  if (!client.registerNode (outdoor1)) {
    Serial.println (F("Cannot register outdoor1 server"));
  }

  if (!client.registerNode (outdoor2)) {
    Serial.println (F("Cannot register outdoor2 server"));
  }

  char *reply;

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

    if (!spx -> read (reply)) {
      Serial.println (F("Read failed"));
    } else {
      Serial.println (reply);
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
}

void loop() {
  delay (5000);

  Serial.println (F("-- SENSORS --"));
  SensoriaIterator iter = client.getIterator ();
  SensorProxy *spx;
  while ((spx = iter.nextSensor ())) {
    Serial.print (spx -> name);
    Serial.print (F(": "));

    char *reply;
    if (!spx -> read (reply)) {
      Serial.println (F("Read failed"));
    } else {
      Serial.println (reply);
    }
  }
  Serial.println (F("-------------"));
}

