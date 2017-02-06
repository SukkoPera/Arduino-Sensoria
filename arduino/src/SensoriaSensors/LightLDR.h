#include <SensoriaCore/Sensor.h>
#include <SensoriaStereotypes/WeatherData.h>

class PhotoSensor: public Sensor<WeatherData> {
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

		return Sensor::begin (name, F("WD"), description, F("20160320"));
	}

  boolean read (WeatherData& wd) override {
		// Assume reading extremes indicates some problem. Debatable, yeah.
		do {
			wd.light10bit = analogRead (pin);
			DPRINT (F("Analog reading = "));
			DPRINTLN (wd.light10bit);
		//~ } while (wd.light10bit == 0 || wd.light10bit == 1023);
		} while (0);

		//~ float v = (float) readVcc () / (float) STEPS * (float) reading / 1000;
		//~ DPRINT (F("Voltage across LDR = "));
		//~ DPRINTLN (v);

		return true;
	}
};
