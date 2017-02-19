#include <SensoriaCore/Sensor.h>
#include <SensoriaStereotypes/WeatherData.h>

/* Uses the Adafruit SHT31 library, install from Library Manager.
 * Seems to work fine with the WeMos SHT30 shield as well, will use D1 as SDA
 * and D2 as SCL.
 */
#include <Adafruit_SHT31.h>


class SHT30HumiditySensor: public Sensor<WeatherData> {
private:
  static const byte DEFAULT_I2C_ADDRESS = 0x45;  // 0x44 is a possible alternative
  Adafruit_SHT31 sensor;

public:
	SHT30HumiditySensor () {
	}

	bool begin (FlashString name, FlashString description, byte i2cAddr = DEFAULT_I2C_ADDRESS) {
		if (Sensor::begin (name, F("WD"), description, F("20170219"))) {
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
    float h = sensor.readHumidity ();
		float t = sensor.readTemperature ();
		if (!isnan (h) && !isnan(t)) {
      wd.humidity = h;
      wd.temperature = t;
		}

		return true;
	}
};
