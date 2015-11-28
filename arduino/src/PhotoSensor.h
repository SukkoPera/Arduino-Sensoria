#include "Sensor.h"

//~ #define VIN 4.23   // Volt
//~ #define RX 3270 // Ohm
#define RX 2170 // Ohm
#define STEPS 1024

class PhotoSensor: public Sensor {
private:
	byte pin;

	// https://code.google.com/p/tinkerit/wiki/SecretVoltmeter
	long readVcc () {
		long result;

		// Read 1.1V reference against AVcc
		ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
		delay (2); // Wait for Vref to settle
		ADCSRA |= _BV(ADSC); // Convert
		while (bit_is_set (ADCSRA, ADSC))
			;
		result = ADCL;
		result |= ADCH << 8;
		result = 1126400L / result; // Back-calculate AVcc in mV

		return result;
	}

public:
	bool begin (const __FlashStringHelper *name, const __FlashStringHelper *description, byte _pin) {
		if (Sensor::begin (name, description, F("20151128"))) {
			pin = _pin;
			return true;
		} else {
			return false;
		}
	}

	char *read (char *buf, const byte size _UNUSED) {
		int reading;

		// Assume reading extremes indicates some problem. Debatable, yeah.
		do {
			reading = analogRead (pin);
		} while (reading == 0 || reading == 1023);

		//~ Serial.print ("Analog reading = ");
		//~ Serial.println (reading);

		float v = (float) readVcc () / (float) STEPS * (float) reading / 1000;
		//~ Serial.print ("Voltage across LDR = ");
		//~ Serial.println (v);

		float rf = (float) reading;
		floatToString (rf, buf);
		int l = strlen (buf);
		buf[l] = ' ';
		floatToString (v, buf + l + 1);

		return buf;
	}
};
