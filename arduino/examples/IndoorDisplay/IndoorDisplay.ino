#include <SensoriaServer.h>

#include <LiquidCrystal_I2C.h>
#include <LcdDisplayActuator.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <DallasTemperatureSensor.h>

#define SSID        ""
#define PASSWORD    ""

//lcd (7, 6, 5, 4, 3, 2)
//lcd (0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
LiquidCrystal_I2C lcd (0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
LcdDisplay lcdActuator;

OneWire oneWire (12);
DallasTemperature sensors (&oneWire);
DallasTemperatureSensor dallasSensor;


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

const char serverVer[] PROGMEM = "20151116";

SensoriaWifiServer srv ("KitchenDisplay", serverVer, 10, 11, SSID, PASSWORD);

void registerTransducer (Transducer& transducer) {
  int b = srv.addTransducer (transducer);
  if (b >= 0) {
    DPRINT (F("Sensor "));
    DPRINT (b);
    DPRINT (F(" registered: "));
    DPRINTLN (transducer.name);

    return;
  } else {
    panic (1000);
  }
}

void setup (void) {
  DSTART ();
  //Serial.begin (9600);

  // Init screen
  lcd.begin (16, 4);
  lcd.clear ();

  lcd.home ();
  lcd.print (F("Init Wi-Fi..."));
  if (!srv.begin ()) {
    panic (500);
  }

  // Init DS18B20

  // locate devices on the bus
  lcd.home   ();
  lcd.print (F("Init Sensor..."));
  sensors.begin ();
  DPRINT (F("Found "));
  DPRINT (sensors.getDeviceCount(), DEC);
  DPRINTLN (F(" device(s)"));

  DeviceAddress insideThermometer;
  if (!sensors.getAddress (insideThermometer, 0)) {
    DPRINTLN (F("Unable to find address for Device 0"));
    panic (333);
  }

#if 0
  DPRINT (F("Device 0 Address: "));
  printAddress (insideThermometer);
  DPRINTLN ();
#endif

  // Set resolution to 9/10/11/12 bits (better precision = slower)
  sensors.setResolution (insideThermometer, 12);

  // Register sensors with server
  lcd.home ();
  lcd.print (F("Register sensors..."));
  lcdActuator.begin (F("LD"), F("LCD Display"), &lcd);
  registerTransducer (lcdActuator);
  dallasSensor.begin (F("IT"), F("Indoor Temperature"), &sensors, insideThermometer);
  registerTransducer (dallasSensor);

  // Signal we're ready!
  lcd.clear ();
  lcd.print (F("Ready!"));
  //Serial.println (F("GO!"));
  pinMode (LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite (LED_BUILTIN, HIGH);
    delay (100);
    digitalWrite (LED_BUILTIN, LOW);
    delay (100);
  }

  delay (500);
  lcd.clear ();
}

void loop (void) {
  srv.receive ();
}

