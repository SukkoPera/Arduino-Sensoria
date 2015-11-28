#include "Sensor.h"

class LightSensorBH1750: public Sensor {
private:
	// https://github.com/claws/BH1750
	BH1750 *lightMeter;

public:
	LightSensorBH1750 () {
		lightMeter = NULL;
	}

	bool begin (const __FlashStringHelper *name, const __FlashStringHelper *description, BH1750& _lightMeter) {
		if (Sensor::begin (name, description, F("20151128"))) {
			lightMeter = &_lightMeter;
			return true;
		} else {
			return false;
		}
	}

	char *read (char *buf, const byte size _UNUSED) {
		uint16_t lux = lightMeter -> readLightLevel ();
		//~ DPRINT (F("Light: "));
		//~ DPRINT (lux);
		//~ DPRINTLN (F(" lx"));

		// Avoid using sprintf
		//snprintf (buf, sizeof (buf), "%hu", lux);
		float f = (float) lux;
		floatToString (f, buf);

		return buf;
	}
};
