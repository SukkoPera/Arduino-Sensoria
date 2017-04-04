#include <Sensoria.h>

#include <SensoriaSensors/LightLDR.h>
#define LDR_PIN A0
#define LDR_RESISTANCE 3260 // Resistance of other resistor in the divider
PhotoSensor photoSensor;

// Communicator & Server
#include <SoftwareSerial.h>
SoftwareSerial swSerial (6, 7);

#include <SensoriaCommunicators/ESPWifiAlt.h>
SensoriaEsp8266Communicator comm;

#include <SensoriaCore/Server.h>
SensoriaServer srv;

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

void setup (void) {
	DSTART (9600);

	// Wait for ESP8266 to init
	delay (1500);

	swSerial.begin (9600);
	if (!comm.begin (swSerial, WIFI_SSID, WIFI_PASSWORD)) {
		mypanic (100);
	}

	if (!srv.begin (F("LightServer"), comm)) {
		mypanic (500);
	}

	// LDR
	pinMode (LDR_PIN, INPUT);
	if (photoSensor.begin (F("OL"), F("Outdoor Light (LDR)"), LDR_PIN,
												 LDR_RESISTANCE)) {
    if (srv.addTransducer (photoSensor) < 0) {
      DPRINTLN (F("Cannot register sensor"));
      mypanic (1000);
    }

    DPRINT (F("Sensor registered: "));
    DPRINTLN (photoSensor.name);
	}

	// Signal we're ready!
	Serial.println (F("GO!"));
	pinMode (LED_PIN, OUTPUT);
	for (int i = 0; i < 3; i++) {
		digitalWrite (LED_PIN, HIGH);
		delay (100);
		digitalWrite (LED_PIN, LOW);
		delay (100);
	}
}

void loop (void) {
	srv.loop ();
}
