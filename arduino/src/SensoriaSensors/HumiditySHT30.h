#include <SensoriaCore/Sensor.h>
#include <SensoriaStereotypes/WeatherData.h>

/* Uses the Adafruit SHT31 library, install from Library Manager.
 * Seems to work fine with the WeMos SHT30 shield as well, will use D1 as SDA
 * and D2 as SCL.
 */
#include <Adafruit_SHT31.h>


class SHT30HumiditySensor: public Sensor<WeatherData> {
private:
  Adafruit_SHT31 sensor;

public:
  static const byte DEFAULT_ADDRESS = 0x44;
  static const byte ALTERNATE_ADDRESS = 0x45;

	SHT30HumiditySensor () {
	}

	bool begin (FlashString name, FlashString description, byte i2cAddr = DEFAULT_ADDRESS) {
		if (Sensor::begin (name, F("WD"), description, F("20171121"))) {
      if (!sensor.begin (i2cAddr)) {
        DPRINTLN (F("SHT3x not found"));
        return false;
      } else {
        return true;
      }
		} else {
			return false;
		}
	}

  boolean read (WeatherData& wd) override {
		boolean ret = false;

    float h = sensor.readHumidity ();
		float t = sensor.readTemperature ();
		if (!isnan (h) && !isnan(t)) {
      wd.humidity = h;
      wd.temperature = t;

      ret = true;
		}

		return ret;
	}
};
