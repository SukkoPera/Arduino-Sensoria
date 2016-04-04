#include <SensoriaCore/Sensor.h>
#include <SensoriaStereotypes/WeatherData.h>
#include <SI7021.h>

class SI7021HumiditySensor: public Sensor {
private:
  SI7021 sensor;

public:
	SI7021HumiditySensor () {
	}

	bool begin (FlashString name, FlashString description) {
		if (Sensor::begin (name, F("WD"), description, F("20160320"))) {
      if (!sensor.begin ()) {
        DPRINTLN (F("SI7021.begin() failed"));
        return false;
      } else {
        return true;
      }
		} else {
			return false;
		}
	}

  boolean read (Stereotype *st) override {
    si7021_env reading = sensor.getHumidityAndTemperature();
  	float h = reading.humidityBasisPoints / 100.0;
		float t = reading.celsiusHundredths / 100.0;
		if (!isnan (h) && !isnan(t)) {
      WeatherData& wd = *static_cast<WeatherData *> (st);
      wd.humidity = h;
      wd.temperature = t;
		}

		return true;
	}
};
