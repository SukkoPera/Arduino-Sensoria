#include <Sensoria.h>

// Digital Light Sensor
//~ #include <BH1750.h>
//~ #include <LightSensorBH1750.h>

//~ BH1750 lightMeter;
//~ LightSensorBH1750 lightSensor1750;

#include <SparkFunTSL2561.h>
#include <SensoriaSensors/LightTSL2561.h>

SFE_TSL2561 lightMeter;
LightSensorTSL2561 lightSensor2561;


// Dallas Temperature Sensor
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SensoriaSensors/TemperatureDallas.h>

OneWire oneWire (7);
DallasTemperature sensors (&oneWire);
DallasTemperatureSensor dallasSensor;


// DHTxx Humidity Sensor
/*#include <DHT.h>
#include <SensoriaSensors/HumidityDHT.h>

DHT dht (2, DHT22);
DhtHumiditySensor dhtSensor;
*/

// SI7021 Humidity Sensor
#include <SensoriaSensors/HumiditySI7021.h>
SI7021HumiditySensor si7021;



// BMP180 Barometric Pressure Sensor
//~ #include <SensoriaSensors/PressureBMP180.h>

//~ SFE_BMP180 pressure;
//~ PressureSensor pressureSensor;

/*
// Simple Photoresistor
#include <SensoriaSensors/LightLDR.h>

#define LDR_PIN A0
#define LDR_RESISTANCE 3240	// Resistance of other resistor in the divider
PhotoSensor photoSensor;
*/

// LM35 Temperature Sensor
#include <SensoriaSensors/TemperatureLM35.h>
#define LM35_PIN A1
TemperatureSensorLM35 lm35Sensor;


// Communicator & Server
#include <SoftwareSerial.h>
SoftwareSerial swSerial (10, 11);

#include <SensoriaCommunicators/ESPWifiAlt.h>
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

  if (!srv.begin (F("Outdoor-2"), comm)) {
    mypanic (500);
  }

  // LDR
#if 0
  pinMode (LDR_PIN, INPUT);
  if (photoSensor.begin (F("L2"), F("Outdoor Light (LDR)"), LDR_PIN, LDR_RESISTANCE))
    registerTransducer (photoSensor);
#endif

  // TSL2561
#if 1
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
    registerTransducer (lightSensor2561);
#endif

  // Init DS18B20
#if 1
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

  if (dallasSensor.begin (F("T2"), F("Outdoor Temperature"), &sensors, outdoorThermometer))
    registerTransducer (dallasSensor);
#endif

#if 0
  dht.begin ();
  if (dhtSensor.begin (F("H2"), F("Outdoor Humidity"), dht))
    registerTransducer (dhtSensor);
#endif

  //~ if (pressure.begin () && pressureSensor.begin (F("P2"), F("Outdoor Pressure"), pressure))
    //~ registerTransducer (pressureSensor);

  if (si7021.begin (F("H3"), F("Outdoor Humid+Temp")))
    registerTransducer (si7021);

  pinMode (LM35_PIN, INPUT);
  if (lm35Sensor.begin (F("T3"), F("Outdoor Temp (LM35)"), LM35_PIN))
    registerTransducer (lm35Sensor);


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
  srv.loop ();
}
