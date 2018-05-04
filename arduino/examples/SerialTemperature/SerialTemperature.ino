#include <Sensoria.h>

// Arduino "Secret" Temperature Sensor
#include <SensoriaSensors/Temperature328.h>
TemperatureSensor328 secretSensor;

#include <SensoriaCommunicators/Serial.h>
SensoriaSerialCommunicator comm;

#include <SensoriaCore/Server.h>
SensoriaServer srv;
const byte MY_ADDR = 0x42;

void mypanic (int interval) {
	pinMode (LED_BUILTIN, OUTPUT);
	while (42) {
		digitalWrite (LED_BUILTIN, HIGH);
		delay (interval);
		digitalWrite (LED_BUILTIN, LOW);
		delay (interval);
	}
}

void setup (void) {
	Serial.begin (9600);
	comm.begin (Serial, MY_ADDR);
	if (!srv.begin (F("Indoor-1"), comm)) {
		mypanic (500);
	}

	if (secretSensor.begin (F("HT"), F("Indoor Temp"))) {
		if (srv.addTransducer (secretSensor) >= 0) {
			DPRINT (F("Sensor registered: "));
			DPRINTLN (secretSensor.name);
		} else {
			mypanic (1000);
		}
	} else {
		DPRINTLN (F("Sensor failed begin()"));
	}

	// We're ready!
	Serial.println (F("GO!"));
}

void loop (void) {
	srv.loop ();
}
