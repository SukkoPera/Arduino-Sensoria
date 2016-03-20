#include <SensoriaCore/Sensor.h>
#include <SensoriaStereotypes/WeatherData.h>
#include <SparkFunTSL2561.h>

class LightSensorTSL2561: public Sensor {
private:
	// Use the SparkFun library
	SFE_TSL2561 *lightMeter;

	// Sensor gain
	boolean gain;

	// Sensor integration time
	unsigned int ms;

	// If there's an I2C error, this function will
	// print out an explanation.
	void printError (byte error) {
		DPRINT ("I2C error: ");
		DPRINT (error, DEC);
		DPRINT (", ");

		switch (error) {
			case 0:
				DPRINTLN ("success");
				break;

			case 1:
				DPRINTLN ("data too long for transmit buffer");
				break;

			case 2:
				DPRINTLN ("received NACK on address (disconnected?)");
				break;

			case 3:
				DPRINTLN ("received NACK on data");
				break;

			case 4:
				DPRINTLN ("other error");
				break;

			default:
				DPRINTLN ("unknown error");
		}
	}

public:
	LightSensorTSL2561 () {
		lightMeter = NULL;
	}

	bool begin (FlashString name, FlashString description, SFE_TSL2561 &_lightMeter, boolean _gain, unsigned int _ms) {
		if (Sensor::begin (name, F("WD"), description, F("20160320"))) {
			lightMeter = &_lightMeter;
			gain = _gain;
			ms = _ms;
			return true;
		} else {
			return false;
		}
	}

  boolean read (Stereotype *st) override {
    WeatherData& wd = *static_cast<WeatherData *> (st);

		unsigned int data0, data1;
		if (lightMeter -> getData (data0, data1)) {
			// getData() returned true, communication was successful
			DPRINT ("data0: = ");
			DPRINT (data0);     // Visibile light
			DPRINT (", data1 = ");
			DPRINT (data1);     // Infrared light?

			// To calculate lux, pass all your settings and readings
			// to the getLux() function.

			// The getLux() function will return 1 if the calculation
			// was successful, or 0 if one or both of the sensors was
			// saturated (too much light). If this happens, you can
			// reduce the integration time and/or gain.
			double lux;    // Resulting lux value
			boolean good;  // True if neither sensor is saturated
			good = lightMeter -> getLux (gain, ms, data0, data1, lux);

			// Print out the results:
			DPRINT (" lux: ");
			DPRINT (lux);
			if (good) {
				DPRINTLN (" (good)");
        wd.lightLux = lux;
        return true;
			} else {
				DPRINTLN (" (BAD)");
			}
		} else {
			// getData() returned false because of an I2C error, inform the user.
			printError (lightMeter -> getError());
    }

    return false;
	}
};
