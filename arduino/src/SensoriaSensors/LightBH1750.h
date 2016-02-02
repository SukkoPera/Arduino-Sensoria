#include <SensoriaInternals/Sensor.h>

class LightSensorBH1750: public Sensor {
private:
	// https://github.com/claws/BH1750
	BH1750 *lightMeter;

public:
	LightSensorBH1750 () {
		lightMeter = NULL;
	}

	bool begin (FlashString name, FlashString description, BH1750& _lightMeter) {
		if (Sensor::begin (name, description, F("20160125"))) {
			lightMeter = &_lightMeter;
			return true;
		} else {
			return false;
		}
	}

	char *read (char *buf, const byte size _UNUSED) {
		uint16_t lux = lightMeter -> readLightLevel ();
		DPRINT (F("Light: "));
		DPRINT (lux);
		DPRINTLN (F(" lx"));

		// Avoid using sprintf
		buf[0] = 'L';
		buf[1] = 'X';
		buf[2] = ':';
		floatToString ((float) lux, buf + 3);

		return buf;
	}
};
