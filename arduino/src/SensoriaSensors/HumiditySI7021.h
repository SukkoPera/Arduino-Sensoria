#include <SensoriaCore/Sensor.h>

#include <SI7021.h>

class SI7021HumiditySensor: public Sensor {
private:
  SI7021 sensor;

public:
	SI7021HumiditySensor () {
	}

	bool begin (FlashString name, FlashString description) {
		if (Sensor::begin (name, description, F("20160214"))) {
      if (!sensor.begin ()) {
        DPRINTLN ("SI7021.begin() failed");
        return false;
      } else {
        return true;
      }
		} else {
			return false;
		}
	}

	char *read (char *buf, const byte size _UNUSED) {
    si7021_env reading = sensor.getHumidityAndTemperature();
  	float h = reading.humidityBasisPoints / 100.0;
		float t = reading.celsiusHundredths / 100.0;
		if (!isnan (h) && !isnan(t)) {
			buf[0] = 'T';
			buf[1] = ':';
			floatToString (t, buf + 2);
			int l = strlen (buf);
			buf[l] = ' ';
			buf[l + 1] = 'H';
			buf[l + 2] = ':';
			floatToString (h, buf + l + 3);
		}

		return buf;
	}
};
