#include <SoftwareSerial.h>
#include <Sensoria.h>
#include <SensoriaClient/SensoriaClient.h>
#include <SensoriaStereotypes/WeatherData.h>
#include <SensoriaCommunicators/ESPWifi.h>

//~ IPAddress outdoor1 (192, 168, 1, 152);

// Wi-Fi parameters
#define WIFI_SSID        "SukkoNet-TO"
#define WIFI_PASSWORD    "everythingyouknowiswrong"

SoftwareSerial swSerial (6, 7);
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

void setup () {
  Serial.begin (9600);

  swSerial.begin (9600);
  if (!comm.begin (swSerial, WIFI_SSID, WIFI_PASSWORD)) {
    mypanic (100);
  }

  client.begin (comm, true);

	UdpAddress* outdoor1 = comm.getAddress (192, 168, 1, 152, 9999);
	while (!client.registerNode (outdoor1)) {
		Serial.println (F("Cannot register outdoor1 node"));
      delay (1000);
	}
  Serial.println (F("Node outdoor1 registered"));
}

void loop () {
  SensorProxy* sensors[] = {
    client.getSensor ("OT"),
    client.getSensor ("OH"),
    client.getSensor ("OP"),
    NULL
  };

  Stereotype *st;
  SensorProxy *sensor;
  for (int i = 0; (sensor = sensors[i]); i++) {
    if (!(sensor -> read (st))) {
      Serial.println (F("Read FAILED"));
    } else {
      Serial.print (F("Reading sensor: "));
      Serial.println (sensor -> name);

      WeatherData& data = *static_cast<WeatherData *> (st);
      if (data.temperature != WeatherData::UNDEFINED) {
        Serial.print (F("Temp = "));
        Serial.println (data.temperature);
      }
      if (data.humidity != WeatherData::UNDEFINED) {
        Serial.print (F("Humidity = "));
        Serial.println (data.humidity);
      }
      if (data.localPressure != WeatherData::UNDEFINED) {
        Serial.print (F("Local Pressure = "));
        Serial.println (data.localPressure);
      }
    }
  }

  client.loop ();

  delay (5000);
}

