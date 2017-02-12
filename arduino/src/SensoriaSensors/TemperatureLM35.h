#include <SensoriaCore/Sensor.h>
#include <SensoriaStereotypes/WeatherData.h>
#include <DallasTemperature.h>

class TemperatureSensorLM35: public Sensor<WeatherData> {
private:
  // How many times to read the sensor, to smooth out inaccuracies
  static const byte N_SAMPLES = 3;

  // How much to wait between two readings (ms)
  static const byte READ_DELAY = 10;

  // DAC resolution
  static const unsigned int STEPS = 1023;

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

		return Sensor::begin (name, F("WD"), description, F("20160320"));
	}

  boolean read (WeatherData& wd) override {
    // Initial delay to let things settle down
    delay (50);

		double val = 0;
		for (int i = 0; i < N_SAMPLES; i++) {
		  val += analogRead (pin);
		  delay (READ_DELAY);
		}
		val /= (float) N_SAMPLES;
		float mv = (float) readVcc () / (float) STEPS * (float) val;
		wd.temperature = mv / 10.0;
		DPRINT (F("Temperature = "));
		DPRINTLN (wd.temperature);

		return true;
	}
};
