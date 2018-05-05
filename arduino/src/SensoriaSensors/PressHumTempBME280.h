#include <SensoriaCore/Sensor.h>
#include <SensoriaStereotypes/WeatherData.h>

// https://github.com/SukkoPera/Arduino-BoschBME280
#include <BoschBME280.h>

class BoschBME280Sensor: public Sensor<WeatherData> {
private:
	BoschBME280 *sensor;

public:
	BoschBME280Sensor (): sensor (NULL) {
	}

	boolean begin (FlashString name, FlashString description, BoschBME280& _sensor) {
		if (Sensor::begin (name, F("WD"), description, F("20180428"))) {
			sensor = &_sensor;
			return true;
		} else {
			return false;
		}
	}

	boolean read (WeatherData& wd) override {
		boolean ret = false;

		if (sensor != NULL) {
			// Start measurements
			if (sensor -> set_sensor_mode (BoschBME280::MODE_FORCED) == 0) {
				// Wait for the measurements to complete
				delay (40);

				BoschBME280::data data;
				if (sensor -> get_sensor_data (BoschBME280::DATA_ALL, data) == 0) {
					// Export measurements
					wd.localPressure = data.pressure / 100.0;		// 100 Pa = 1 mbar
					//~ wd.temperature = data.temperature / 100.0;
					//~ wd.humidity = data.humidity / 1024.0;
					wd.temperature = data.temperature;
					wd.humidity = data.humidity;

#if 0
					// Print out the measurements:
					DPRINT (F("Temperature: "));
					DPRINT (wd.temperature, 2);
					DPRINTLN (F("*C"));

					DPRINT (F("Humidity: "));
					DPRINT (wd.humidity, 2);
					DPRINTLN (F("%"));

					DPRINT (F("Absolute pressure: "));
					DPRINT (wd.localPressure, 2);
					DPRINTLN (F(" millibar"));
#endif

					ret = true;
				} else {
					DPRINTLN (F("Error retrieving measurement"));
				}
			} else {
				DPRINTLN (F("Error starting measurement"));
			}
		} else {
			DPRINTLN (F("sensor is NULL"));
		}

		return ret;
	}
};
