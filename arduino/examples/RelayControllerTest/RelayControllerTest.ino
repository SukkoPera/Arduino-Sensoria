#include <Sensoria.h>
#include <SensoriaStereotypes/WeatherData.h>

#include <SensoriaCore/Server.h>
SensoriaServer srv;

#include <SoftwareSerial.h>
#include <SensoriaCommunicators/ESPWifi.h>
SoftwareSerial swSerial (6, 7);
SensoriaEsp8266Communicator comm;

#include <SensoriaActuators/ControlledRelay.h>
ControlledRelay relay;

#include <SensoriaControllers/RelayController.h>
OverTempController<20> rc;

#include <SensoriaClient/SensoriaClient.h>
SensoriaClient client;

#include <SensoriaClient/NotificationManager.h>
NotificationManager notMgr;



IPAddress outdoor1 (192, 168, 1, 154);

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

void setup () {
  Serial.begin (9600);

  swSerial.begin (9600);
  if (!comm.begin (swSerial, WIFI_SSID, WIFI_PASSWORD, CC_SERVER | CC_NOTIFICATIONS)) {
    mypanic (100);
  }

  // Init server
  if (!srv.begin (F("RelayControllerTest"), comm)) {
    mypanic (500);
  }

  // Register relay with server
  if (relay.begin (F("RR"), F("Controlled Relay"), 9)) {
    if (srv.addTransducer (relay) >= 0) {
      DPRINT (F("Actuator registered: "));
      DPRINTLN (relay.name);
    } else {
      mypanic (1000);
    }
  } else {
    DPRINTLN (F("Actuator failed begin()"));
  }

  // Init client
  client.begin (comm);

  // Register temperature sensor
	while (!client.registerNode (outdoor1)) {
		Serial.println (F("Cannot register Outdoor-1 node"));
    delay (1000);
	}
  Serial.println (F("Node Outdoor-1 registered"));

  // Init Notification Manager
  notMgr.begin (comm);

  // Register Relay Controller (Notification Processor)
  SensorProxy* ot = client.getSensor ("TK");
  if (ot != NULL) {
    rc.begin (relay, *ot);
    notMgr.registerReceiver (rc, *ot, NT_PRD, 10);
    //~ notMgr.registerReceiver (rc, *ot, NT_CHA);
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
  notMgr.loop ();
}
