#include <Sensoria.h>

#include <SensoriaCommunicators/CommEthernet.h>
SensoriaEthernetCommunicator comm;
byte mac[6] = {0xDE, 0xAD, 0xBE, 0xEE, 0xEE, 0xEF};

#include <SensoriaCore/Server.h>
SensoriaServer srv;

#include <BoschBME280.h>
BoschBME280 bme;

#include <SensoriaSensors/PressHumTempBME280.h>
BoschBME280Sensor sensor;

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

	Serial.println (F("Initializing..."));

	comm.begin (mac);

	if (!srv.begin (F("TestBME280"), comm)) {
		mypanic (500);
	}

	// Init sensor, recommended mode of operation: Indoor navigation
	BoschBME280::Settings settings;
	settings.osr_h = BoschBME280::OVERSAMPLING_1X;
	settings.osr_p = BoschBME280::OVERSAMPLING_16X;
	settings.osr_t = BoschBME280::OVERSAMPLING_2X;
	settings.filter = BoschBME280::FILTER_COEFF_16;
	if (bme.begin (BoschBME280::I2C_ADDR_PRIM, BoschBME280::INTF_I2C, settings) == 0) {
		uint8_t settings_sel = BoschBME280::SEL_OSR_PRESS | BoschBME280::SEL_OSR_TEMP | BoschBME280::SEL_OSR_HUM | BoschBME280::SEL_FILTER;
		if (bme.set_sensor_settings (settings_sel) == 0) {
			if (sensor.begin (F("TT"), F("BME280 T/H/P"), bme)) {
				if (srv.addTransducer (sensor) >= 0) {
					DPRINT (F("Sensor registered: "));
					DPRINTLN (sensor.name);
				} else {
					mypanic (1000);
				}
			} else {
				DPRINTLN ("Sensor failed begin()");
			}

			// Signal we're ready!
			Serial.println (F("GO!"));
			pinMode (LED_PIN, OUTPUT);
			for (int i = 0; i < 3; i++) {
				digitalWrite (LED_PIN, LOW);		// Led uses inverse logic
				delay (100);
				digitalWrite (LED_PIN, HIGH);
				delay (100);
			}
		} else {
			mypanic (100);
		}
	} else {
		mypanic (333);
	}
}

void loop (void) {
	srv.loop ();
}
