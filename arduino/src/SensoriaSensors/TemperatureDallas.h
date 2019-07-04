#include <SensoriaCore/Sensor.h>
#include <SensoriaStereotypes/WeatherData.h>
#include <DallasTemperature.h>

class DallasTemperatureSensor: public Sensor<WeatherData> {
private:
	DallasTemperature *sensors;
	DeviceAddress sensorAddress;

public:
	boolean begin (FlashString name, FlashString description, DallasTemperature *_sensors, DeviceAddress _sensorAddress) {
		if (_sensors != NULL && Sensor::begin (name, F("WD"), description)) {
			sensors = _sensors;

			// Maybe the DallasTemperature library should provide a copyAddress() method...
			for (byte b = 0; b < sizeof (DeviceAddress); b++)
				sensorAddress[b] = _sensorAddress[b];
			return true;
		} else {
			return false;
		}
	}

  boolean read (WeatherData& wd) override {
		DPRINT (F("Requesting temperatures... "));
		sensors -> requestTemperatures (); // Send the command to get temperatures
		DPRINTLN (F("DONE"));

		wd.temperature = sensors -> getTempC (sensorAddress);
		DPRINT (F("Temp C: "));
		DPRINTLN (wd.temperature);

		return true;
	}
};
