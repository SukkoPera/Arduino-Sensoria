#include <SensoriaCore/Sensor.h>
#include <SensoriaStereotypes/WeatherData.h>

class LightSensorBH1750: public Sensor<WeatherData> {
private:
	// https://github.com/claws/BH1750
	BH1750 *lightMeter;

public:
	LightSensorBH1750 ():	lightMeter (NULL) {
	}

	bool begin (FlashString name, FlashString description, BH1750& _lightMeter) {
		if (Sensor::begin (name, F("WD"), description, F("20160320"))) {
			lightMeter = &_lightMeter;
			return true;
		} else {
			return false;
		}
	}

  boolean read (WeatherData& wd) override {
		wd.lightLux = lightMeter -> readLightLevel ();
		DPRINT (F("Light: "));
		DPRINT (wd.lightLux);
		DPRINTLN (F(" lx"));

		return true;
	}
};
