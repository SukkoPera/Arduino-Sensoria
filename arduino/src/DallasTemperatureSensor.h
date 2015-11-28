#include <DallasTemperature.h>
#include "Sensor.h"
#include "internals/debug.h"

class DallasTemperatureSensor: public Sensor {
private:
	DallasTemperature *sensors;
	DeviceAddress sensorAddress;

public:
	bool begin (const __FlashStringHelper *name, const __FlashStringHelper *description, DallasTemperature *_sensors, DeviceAddress _sensorAddress) {
		if (_sensors != NULL && Sensor::begin (name, description, F("20151128"))) {
			sensors = _sensors;

			// Maybe the DallasTemperature library should provide a copyAddress() method...
			for (byte b = 0; b < sizeof (DeviceAddress); b++)
				sensorAddress[b] = _sensorAddress[b];
			return true;
		} else {
			return false;
		}
	}

	char *read (char *buf, const byte size _UNUSED) {
		DPRINT (F("Requesting temperatures... "));
		sensors -> requestTemperatures (); // Send the command to get temperatures
		DPRINTLN (F("DONE"));

		float tempC = sensors -> getTempC (sensorAddress);
		DPRINT (F("Temp C: "));
		DPRINTLN (tempC);

		// This doesn't work... why?
		//snprintf (buf, DALLAS_BUF_SIZE, "%f", tempC);
		floatToString (tempC, buf);

		return buf;
	}
};
