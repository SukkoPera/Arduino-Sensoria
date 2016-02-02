#include <SensoriaInternals/Sensor.h>

class PhotoSensor: public Sensor {
private:
	byte pin;
	unsigned int resistance;

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
	bool begin (FlashString name, FlashString description, byte _pin, unsigned int _resistance) {
		pin = _pin;
		resistance = _resistance;

		return Sensor::begin (name, description, F("20160125"));
	}

	char *read (char *buf, const byte size _UNUSED) {
		int reading;

		// Assume reading extremes indicates some problem. Debatable, yeah.
		do {
			reading = analogRead (pin);
			DPRINT ("Analog reading = ");
			DPRINTLN (reading);
		} while (reading == 0 || reading == 1023);

		//~ float v = (float) readVcc () / (float) STEPS * (float) reading / 1000;
		//~ DPRINT ("Voltage across LDR = ");
		//~ DPRINTLN (v);

		buf[0] = 'L';
		buf[1] = 'U';
		buf[2] = ':';
		float rf = (float) reading;
		floatToString (rf, buf + 3);
		//~ int l = strlen (buf);
		//~ buf[l] = ' ';
		//~ floatToString (v, buf + l + 1);

		return buf;
	}
};
