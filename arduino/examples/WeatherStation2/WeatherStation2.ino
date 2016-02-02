#include <SensoriaServer.h>

// Digital Light Sensor
//~ #include <BH1750.h>
//~ #include <LightSensorBH1750.h>

//~ BH1750 lightMeter;
//~ LightSensorBH1750 lightSensor1750;

#include <Wire.h>
#include <SparkFunTSL2561.h>
#include <LightSensorTSL2561.h>

SFE_TSL2561 lightMeter;
LightSensorTSL2561 lightSensor2561;


// Dallas Temperature Sensor
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DallasTemperatureSensor.h>

OneWire oneWire (7);
DallasTemperature sensors (&oneWire);
DallasTemperatureSensor dallasSensor;


// DHTxx Humidity Sensor
#include <DHT.h>
#include <DhtHumiditySensor.h>

DHT dht (2, DHT22);
DhtHumiditySensor dhtSensor;


// BMP180 Barometric Pressure Sensor
#include <PressureSensor.h>

SFE_BMP180 pressure;
PressureSensor pressureSensor;

// Simple Photoresistor
#include <PhotoSensor.h>

#define LDR_PIN A0
#define LDR_RESISTANCE 3240	// Resistance of other resistor in the divider
PhotoSensor photoSensor;

// LM35 Temperature Sensor
#include <TemperatureSensorLM35.h>
#define LM35_PIN A1
TemperatureSensorLM35 lm35Sensor;

// Wi-Fi parameters
#define SSID        ""
#define PASSWORD    ""


void panic (int interval) {
  pinMode (LED_BUILTIN, OUTPUT);
  while (42) {
    digitalWrite (LED_BUILTIN, HIGH);
    delay (interval);
    digitalWrite (LED_BUILTIN, LOW);
    delay (interval);
  }
}


#include <WifiServer.h>
SensoriaWifiServer srv (10, 11);


//#include <SerialServer.h>
//SensoriaSerialServer srv;


void registerSensor (Sensor& sensor) {
  int b = srv.addTransducer (sensor);
  if (b >= 0) {
    DPRINT (F("Sensor "));
    DPRINT (b);
    DPRINT (F(" registered: "));
    DPRINTLN (sensor.name);

    return;
  } else {
    panic (1000);
  }
}

void setup (void) {
  DSTART ();

  if (!srv.begin (F("Outdoor-2"), SSID, PASSWORD)) {
    panic (500);
  }

  pinMode (LDR_PIN, INPUT);
  if (photoSensor.begin (F("L2"), F("Outdoor Light (LDR)"), LDR_PIN, LDR_RESISTANCE))
    registerSensor (photoSensor);

  // TSL2561
  lightMeter.begin ();
  // If gain = false (0), device is set to low gain (1X)
  // If gain = high (1), device is set to high gain (16X)
  boolean gain = 0;

  // If time = 0, integration will be 13.7ms
  // If time = 1, integration will be 101ms
  // If time = 2, integration will be 402ms
  // If time = 3, use manual start / stop to perform your own integration
  unsigned char time = 2;

  unsigned int ms;  // Integration ("shutter") time in milliseconds
  lightMeter.setTiming (gain, time, ms);
  lightMeter.setPowerUp();
  if (lightSensor2561.begin (F("OR"), F("Outdoor Light (Lux)"), lightMeter, gain, ms))
    registerSensor (lightSensor2561);


  // Init DS18B20

  // locate devices on the bus
  sensors.begin ();
  DPRINT (F("Found "));
  DPRINT (sensors.getDeviceCount(), DEC);
  DPRINTLN (F(" device(s)"));

  DeviceAddress outdoorThermometer;
  if (!sensors.getAddress (outdoorThermometer, 0)) {
    DPRINTLN (F("Unable to find address for Device 0"));
    panic (333);
  }

#if 0
  DPRINT (F("Device 0 Address: "));
  printAddress (insideThermometer);
  DPRINTLN ();
#endif

  // Set resolution to 9/10/11/12 bits (better precision = slower)
  sensors.setResolution (outdoorThermometer, 12);

  if (dallasSensor.begin (F("T2"), F("Outdoor Temperature"), &sensors, outdoorThermometer))
    registerSensor (dallasSensor);


  dht.begin ();
  if (dhtSensor.begin (F("H2"), F("Outdoor Humidity"), dht))
    registerSensor (dhtSensor);


  if (pressure.begin () && pressureSensor.begin (F("P2"), F("Outdoor Pressure"), pressure))
    registerSensor (pressureSensor);

  pinMode (LM35_PIN, INPUT);
  if (lm35Sensor.begin (F("T3"), F("Outdoor Temp (LM35)"), LM35_PIN))
    registerSensor (lm35Sensor);


  // Signal we're ready!
  //Serial.println (F("GO!"));
  pinMode (LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite (LED_BUILTIN, HIGH);
    delay (100);
    digitalWrite (LED_BUILTIN, LOW);
    delay (100);
  }
}

void loop (void) {
  srv.receive ();
}
