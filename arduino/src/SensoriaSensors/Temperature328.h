#include <SensoriaCore/Sensor.h>
#include <SensoriaStereotypes/WeatherData.h>

class TemperatureSensor328: public Sensor {
private:
  /* Read ATmega temperature sensor (only available on 328 and 32u4)
   * http://playground.arduino.cc/Main/InternalTemperatureSensor
   */
  double readTemp () {
    ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
    delay (10); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Convert
    delay (20);            // wait for voltages to become stable.
    while (bit_is_set (ADCSRA, ADSC))
      ;
    // Reading register "ADCW" takes care of how to read ADCL and ADCH.
    return (ADCW - 324.31) / 1.22;
  }

public:
	boolean begin (FlashString name, FlashString description) {
		return Sensor::begin (name, F("WD"), description, F("20160201"));
	}

  boolean read (Stereotype *st) override {
    WeatherData& wd = *static_cast<WeatherData *> (st);
    wd.temperature = readTemp ();
    return true;
  }
};
