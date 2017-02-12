#include <SensoriaCore/Sensor.h>
#include <SensoriaStereotypes/WeatherData.h>
#include <SI7021.h>

class SI7021HumiditySensor: public Sensor<WeatherData> {
private:
  SI7021 sensor;

public:
	SI7021HumiditySensor () {
	}

	bool begin (FlashString name, FlashString description) {
		if (Sensor::begin (name, F("WD"), description, F("20160320"))) {
#ifdef ARDUINO_ARCH_ESP8266
      if (!sensor.begin (D3, D4)) {
#else
      if (!sensor.begin ()) {
#endif
        DPRINTLN (F("SI7021.begin() failed"));
        return false;
      } else {
        return true;
      }
		} else {
			return false;
		}
	}

  boolean read (WeatherData& wd) override {
    si7021_env reading = sensor.getHumidityAndTemperature();
  	float h = reading.humidityBasisPoints / 100.0;
		float t = reading.celsiusHundredths / 100.0;
		if (!isnan (h) && !isnan(t)) {
      wd.humidity = h;
      wd.temperature = t;
		}

		return true;
	}
};
