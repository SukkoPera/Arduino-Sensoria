/* To make this fit on a Nano, as of 04/06/2018:
 * - AllStereotypes.{h,cpp}: Comment out all stereotypes except WD;
 * - common.h: Do not enable ENABLE_NOTIFICATIONS;
 * - Server.h: Set OUT_BUF_SIZE = 144;
 * - ESPWifi.h: Set IN_BUF_SIZE = 128 and N_ADDRESSES = 4.
 *
 * This results in 27804 byte (86%) flash and 1629 byte (79%) RAM with Arduino
 * 1.8.5 and seems to work reliably, even though the HLO reply gets partially
 * cut (i.e.: "Outdoor Pr").
 *
 * Probably it would be better to set IN_BUF_SIZE = 64 (or even 32, we only get
 * short REA commands here) and leave OUT_BUF_SIZE = 192.
 */

#include <Sensoria.h>

// Digital Light Sensor
#include <BH1750.h>
#include <SensoriaSensors/LightBH1750.h>

BH1750 lightMeter;
LightSensorBH1750 lightSensor1750;


// Dallas Temperature Sensor
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SensoriaSensors/TemperatureDallas.h>

OneWire oneWire (7);
DallasTemperature sensors (&oneWire);
DallasTemperatureSensor dallasSensor;


// DHT11 Humidity Sensor
#include <DHT.h>
#include <SensoriaSensors/HumidityDHT.h>

DHT dht (8, DHT22);
DhtHumiditySensor dhtSensor;


// BMP180 Barometric Pressure Sensor
#include <SensoriaSensors/PressureBMP180.h>

SFE_BMP180 pressure;
PressureSensor pressureSensor;

// Simple Photoresistor
#include <SensoriaSensors/LightLDR.h>

#define LDR_PIN A0
#define LDR_RESISTANCE 3260 // Resistance of other resistor in the divider
PhotoSensor photoSensor;

// Communicator & Server
#include <SoftwareSerial.h>
SoftwareSerial swSerial (10, 11);

#include <SensoriaCommunicators/ESPWifi.h>
SensoriaEsp8266Communicator comm;

#include <SensoriaCore/Server.h>
SensoriaServer srv;

// Wi-Fi parameters
#define WIFI_SSID        "ssid"
#define WIFI_PASSWORD    "password"


void mypanic (int interval) {
	pinMode (LED_BUILTIN, OUTPUT);

	while (42) {
		digitalWrite (LED_BUILTIN, HIGH);
		delay (interval);
		digitalWrite (LED_BUILTIN, LOW);
		delay (interval);
	}
}

void registerTransducer (Transducer& sensor) {
  int b = srv.addTransducer (sensor);
  if (b >= 0) {
    DPRINT (F("Sensor "));
    DPRINT (b);
    DPRINT (F(" registered: "));
    DPRINTLN (sensor.name);

    return;
  } else {
    mypanic (1000);
  }
}

void setup (void) {
	DSTART (9600);

  // Wait for ESP8266 to init
	delay (3000);

	swSerial.begin (9600);
	if (!comm.begin (swSerial, WIFI_SSID, WIFI_PASSWORD)) {
		mypanic (100);
	}

	if (!srv.begin (F("Outdoor-1"), comm)) {
		mypanic (500);
	}

	// LDR
	pinMode (LDR_PIN, INPUT);
	if (photoSensor.begin (F("PR"), F("Outdoor Light (LDR)"), LDR_PIN,
	                       LDR_RESISTANCE)) {
		registerTransducer (photoSensor);
	}

	/*
	  Connection:
	   VCC-5v
	   GND-GND
	   SCL-SCL(analog pin 5)
	   SDA-SDA(analog pin 4)
	   ADD-NC or GND
	  */

	lightMeter.begin ();
	if (lightSensor1750.begin (F("OL"), F("Outdoor Light (Lux)"), lightMeter)) {
		registerTransducer (lightSensor1750);
	}


	// Init DS18B20
	// locate devices on the bus
	sensors.begin ();
	DPRINT (F("Found "));
	DPRINT (sensors.getDeviceCount(), DEC);
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

	if (dallasSensor.begin (F("OT"), F("Outdoor Temperature"), &sensors,
	                        outdoorThermometer)) {
		registerTransducer (dallasSensor);
	}


	dht.begin ();
	if (dhtSensor.begin (F("OH"), F("Outdoor Humidity"), dht)) {
		registerTransducer (dhtSensor);
	}


  if (pressure.begin () && pressureSensor.begin (F("OP"), F("Outdoor Pressure"), pressure))
    registerTransducer (pressureSensor);

	// Signal we're ready!
	//Serial.println (F("GO!"));
	pinMode (LED_BUILTIN, OUTPUT);

	for (byte i = 0; i < 3; i++) {
		digitalWrite (LED_BUILTIN, HIGH);
		delay (100);
		digitalWrite (LED_BUILTIN, LOW);
		delay (100);
	}
}

void loop (void) {
	srv.loop ();
}
