#include <SensoriaInternals/Sensor.h>
#include <DallasTemperature.h>

class DallasTemperatureSensor: public Sensor {
private:
	DallasTemperature *sensors;
	DeviceAddress sensorAddress;

public:
	bool begin (FlashString name, FlashString description, DallasTemperature *_sensors, DeviceAddress _sensorAddress) {
		if (_sensors != NULL && Sensor::begin (name, description, F("20160125"))) {
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

		buf[0] = 'T';
		buf[1] = ':';
		floatToString (tempC, buf + 2);

		return buf;
	}
};
