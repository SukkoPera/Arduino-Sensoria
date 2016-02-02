#include "Sensor.h"

class TemperatureSensor328: public Sensor {
private:
  /* Read ATmega temperature sensor (only available on 328 and 32u4)
   * http://playground.arduino.cc/Main/InternalTemperatureSensor
   */
  double readTemp () {
    ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
    delay (5); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Convert
    delay (20);            // wait for voltages to become stable.
    while (bit_is_set (ADCSRA, ADSC))
      ;
    // Reading register "ADCW" takes care of how to read ADCL and ADCH.
    return (ADCW - 324.31) / 1.22;
  }

public:
	bool begin (FlashString name, FlashString description) {
		return Sensor::begin (name, description, F("20160201"));
	}

	char *read (char *buf, const byte size _UNUSED) {
    double temp = readTemp ();
		DPRINT ("Temperature = ");
		DPRINTLN (temp);

		buf[0] = 'T';
		buf[1] = ':';
		floatToString (temp, buf + 2);

		return buf;
	}
};
