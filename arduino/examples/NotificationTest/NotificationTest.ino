#include <SoftwareSerial.h>
#include <Sensoria.h>
#include <SensoriaClient/SensoriaClient.h>
#include <SensoriaClient/NotificationManager.h>
#include <SensoriaStereotypes/WeatherData.h>
#include <SensoriaCommunicators/ESPWifi.h>

IPAddress outdoor1 (192, 168, 1, 154);

// Wi-Fi parameters
#define WIFI_SSID        "ssid"
#define WIFI_PASSWORD    "password"

SoftwareSerial swSerial (6, 7);
SensoriaEsp8266Communicator comm;
SensoriaClient client;

NotificationManager notMgr;

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


class WeatherDataNotificationProcessor: public NotificationReceiver<WeatherData> {
protected:
  boolean onNotification (WeatherData& data) {
    Serial.println (F("Received WeatherData notification:"));
    if (data.temperature != WeatherData::UNDEFINED) {
      Serial.print (F("- Temp = "));
      Serial.println (data.temperature);
    }
    if (data.humidity != WeatherData::UNDEFINED) {
      Serial.print (F("- Humidity = "));
      Serial.println (data.humidity);
    }
    if (data.localPressure != WeatherData::UNDEFINED) {
      Serial.print (F("- Local Pressure = "));
      Serial.println (data.localPressure);
    }

    return true;
  }
};

WeatherDataNotificationProcessor wdProc;
int registrationId = -1;

void setup () {
  Serial.begin (9600);

  swSerial.begin (9600);
  if (!comm.begin (swSerial, WIFI_SSID, WIFI_PASSWORD)) {
    mypanic (100);
  }

  client.begin (comm);

	while (!client.registerNode (outdoor1)) {
		Serial.println (F("Cannot register Outdoor-1 node"));
    delay (1000);
	}
  Serial.println (F("Node Outdoor-1 registered"));

  // Init Notification Manager
  notMgr.begin (comm);

  // Register Notification Processor
  //~ SensorProxy* ot = client.getTransducer ("OT");
  TransducerProxy* ot = client.getTransducer ("KF");
  if (ot != NULL) {
    //~ notMgr.registerReceiver (wdProc, *ot, NT_PRD, 10);
    registrationId = notMgr.registerReceiver (wdProc, *ot, NT_CHA);
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

void loop () {
  client.loop ();
  notMgr.loop ();

  if (!notMgr.isRegistered (registrationId) && (millis () / 500) % 2 == 0) {
    digitalWrite (LED_PIN, LOW);
  } else {
    digitalWrite (LED_PIN, HIGH);
  }

  static unsigned long t = 0;
  if (t == 0 || millis () - t >= 5000) {
    TransducerProxy* ot = client.getTransducer ("KF");
    if (ot != NULL) {
      DPRINTLN ("Reading");
      Stereotype* st;
      ot -> read (st);
      t = millis ();
    }
  }
}
