#include "Sensor.h"

// How many times to read the sensor, to smooth out inaccuracies
#define N_SAMPLES 5

// How much to wait between two readings (ms)
#define READ_DELAY 5

#define STEPS_LM35 1023

class TemperatureSensorLM35: public Sensor {
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
	bool begin (FlashString name, FlashString description, byte _pin) {
		pin = _pin;

		return Sensor::begin (name, description, F("20160201"));
	}

	char *read (char *buf, const byte size _UNUSED) {
		double val = 0;
		for (int i = 0; i < N_SAMPLES; i++) {
		  val += analogRead (pin);
		  delay (READ_DELAY);
		}
		val /= (float) N_SAMPLES;
		float mv = (float) readVcc () / (float) STEPS_LM35 * (float) val;
		float cel = mv / 10;
		DPRINT ("Temperature = ");
		DPRINTLN (cel);

		buf[0] = 'T';
		buf[1] = ':';
		floatToString (cel, buf + 2);

		return buf;
	}
};
