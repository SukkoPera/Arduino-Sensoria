#include <Sensoria.h>
<<<<<<< HEAD
=======
#include <SensoriaServers/ESPStandAlone.h>
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9

#include <OneWire.h>
#include <DallasTemperature.h>
#include <SensoriaSensors/TemperatureDallas.h>

OneWire oneWire (D2);
DallasTemperature sensors (&oneWire);
DallasTemperatureSensor dallasSensor;

<<<<<<< HEAD

// Communicator & Server
#include <SoftwareSerial.h>
SoftwareSerial swSerial (10, 11);

#include <SensoriaCommunicators/ESPStandAlone.h>
ESPCommunicator comm;

#include <SensoriaInternals/Server.h>
SensoriaServer srv;

// Wi-Fi parameters
#define WIFI_SSID        ""
#define WIFI_PASSWORD    ""


void mypanic (int interval) {
  pinMode (LED_BUILTIN, OUTPUT);
  while (42) {
    digitalWrite (LED_BUILTIN, HIGH);
    delay (interval);
    digitalWrite (LED_BUILTIN, LOW);
=======
// Wi-Fi parameters
#define SSID        ""
#define PASSWORD    ""

// Pin 2 seems to control the ESP module led on my NodeMCU
#define LED_PIN BUILTIN_LED

ESPServer srv;


void mypanic (int interval) {
  pinMode (LED_PIN, OUTPUT);
  while (42) {
    digitalWrite (LED_PIN, HIGH);
    delay (interval);
    digitalWrite (LED_PIN, LOW);
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
    delay (interval);
  }
}

void setup (void) {
  DSTART ();

<<<<<<< HEAD
  if (!comm.begin (WIFI_SSID, WIFI_PASSWORD)) {
    mypanic (100);
  }

  if (!srv.begin (F("ESPDallas-1"), comm)) {
    mypanic (500);
  }


=======
  if (!srv.begin (F("ESPDallas"), SSID, PASSWORD)) {
    mypanic (500);
  }

>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
  // Init DS18B20
  // locate devices on the bus
  sensors.begin ();
  DPRINT (F("Found "));
  DPRINT (sensors.getDeviceCount (), DEC);
  DPRINTLN (F(" device(s)"));

  DeviceAddress outdoorThermometer;
  if (!sensors.getAddress (outdoorThermometer, 0)) {
    DPRINTLN (F("Unable to find address for Device 0"));
    mypanic (333);
  }

#if 0
  DPRINT (F("Device 0 Address: "));
  printAddress (insideThermometer);
  DPRINTLN ();
#endif

  // Set resolution to 9/10/11/12 bits (better precision = slower)
  sensors.setResolution (outdoorThermometer, 12);

  if (dallasSensor.begin (F("IT"), F("Indoor Temperature"), &sensors, outdoorThermometer)) {
    if (srv.addTransducer (dallasSensor) >= 0) {
      DPRINT (F("Sensor registered: "));
      DPRINTLN (dallasSensor.name);
    } else {
      mypanic (1000);
    }
  } else {
    DPRINTLN ("Sensor failed begin()");
  }

  // Signal we're ready!
  //Serial.println (F("GO!"));
<<<<<<< HEAD
  pinMode (LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite (LED_BUILTIN, HIGH);
    delay (100);
    digitalWrite (LED_BUILTIN, LOW);
=======
  pinMode (LED_PIN, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite (LED_PIN, HIGH);
    delay (100);
    digitalWrite (LED_PIN, LOW);
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
    delay (100);
  }
}

void loop (void) {
<<<<<<< HEAD
  srv.loop ();
=======
  srv.receive ();
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
}
