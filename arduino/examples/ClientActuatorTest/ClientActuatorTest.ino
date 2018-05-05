#include <SoftwareSerial.h>
#include <Sensoria.h>
#include <SensoriaClient/SensoriaClient.h>
#include <SensoriaStereotypes/ControlledRelayData.h>
#include <SensoriaCommunicators/ESPWifi.h>

IPAddress relayNodeAddr (192, 168, 1, 157);

// Wi-Fi parameters
#define WIFI_SSID        "ssid"
#define WIFI_PASSWORD    "password"

#define RELAY_ACTNAME "LO"

SoftwareSerial swSerial (6, 7);
SensoriaEsp8266Communicator comm;
SensoriaClient client;


ActuatorProxy *relayActuator = NULL;
boolean relayState;


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

  //~ Serial3.begin (9600);
  swSerial.begin (9600);
	if (!comm.begin (swSerial, WIFI_SSID, WIFI_PASSWORD)) {
		Serial.println (F("WiFi error"));
		mypanic (100);
	}

  client.begin (comm);
	while (!client.registerNode (relayNodeAddr)) {
		Serial.println (F("Cannot register Relay Node"));
    delay (1000);
	}
  Serial.println (F("Relay Node registered"));

	relayActuator = client.getActuator (RELAY_ACTNAME);
	if (!relayActuator) {
		mypanic (2000);
	}
  Stereotype *st;
  if (!relayActuator -> read (st)) {
		Serial.println (F("Relay state read failed"));
	} else {
		ControlledRelayData &data = *static_cast<ControlledRelayData *> (st);
    relayState = data.state == ControlledRelayData::STATE_ON;
    Serial.print (F("Initial relay state = "));
    Serial.println (relayState ? F("ON") : F("OFF"));
	}
}

void loop () {
  delay (3000);
  Serial.println ("Toggling relay");
  toggleRelay ();
}


//~ void toggleRelay () {
  //~ // Toggle remote relay
  //~ RelayData rd;
  //~ if (relayState) {
    //~ rd.state = RelayData::STATE_OFF;
  //~ } else {
    //~ rd.state = RelayData::STATE_ON;
  //~ }
  //~ if (relayActuator -> write (rd)) {
    //~ relayState = !relayState;
    //~ Serial.print (F("Relay is now: "));
    //~ Serial.println (relayState ? F("ON") : F("OFF"));
  //~ } else {
    //~ Serial.print (F("Cannot toggle, relay is still: "));
    //~ Serial.println (relayState ? F("ON") : F("OFF"));
  //~ }
//~ }

void toggleRelay () {
  ControlledRelayData rd;
  if (relayState) {
    rd.state = ControlledRelayData::STATE_OFF;
    rd.controller = ControlledRelayData::CTRL_MANUAL;
  } else {
    rd.state = ControlledRelayData::STATE_ON;
    rd.controller = ControlledRelayData::CTRL_MANUAL;
  }
  if (relayActuator -> write (rd)) {
    relayState = !relayState;
    Serial.print (F("Relay is now: "));
    Serial.println (relayState ? F("ON") : F("OFF"));
  } else {
    Serial.print (F("Cannot toggle, relay is still: "));
    Serial.println (relayState ? F("ON") : F("OFF"));
  }
}
