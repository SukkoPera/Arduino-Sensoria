#include <SensoriaCore/Sensor.h>

// https://github.com/sparkfun/BMP180_Breakout/
#include <SFE_BMP180.h>
#include <Wire.h>

#define ALTITUDE 236.0

class PressureSensor: public Sensor {
private:
	SFE_BMP180 *sensor;

public:
	PressureSensor () {
		sensor = NULL;
	}

	bool begin (FlashString name, FlashString description, SFE_BMP180& _sensor) {
		if (Sensor::begin (name, description, F("20160125"))) {
			sensor = &_sensor;
			return true;
		} else {
			return false;
		}
	}

	// FIXME: Check for sensor != null
	char *read (char *buf, const byte size _UNUSED) {
		char status;
		double temp, px;

		// Start a temperature measurement:
		// If request is successful, the number of ms to wait is returned.
		// If request is unsuccessful, 0 is returned.
		status = sensor -> startTemperature ();
		if (status != 0) {
			// Wait for the measurement to complete:
			delay (status);

			// Retrieve the completed temperature measurement:
			// Note that the measurement is stored in the variable T.
			// Function returns 1 if successful, 0 if failure.
			status = sensor -> getTemperature (temp);
			if (status != 0) {
				// Print out the measurement:
				DPRINT (F("Temperature: "));
				DPRINT (temp, 2);
				DPRINTLN (F(" C"));
				//Serial.print ((9.0/5.0)*T+32.0,2);
				//Serial.println (" deg F");

				// Start a pressure measurement:
				// The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
				// If request is successful, the number of ms to wait is returned.
				// If request is unsuccessful, 0 is returned.
				status = sensor -> startPressure (3);
				if (status != 0) {
					// Wait for the measurement to complete:
					delay (status);

					// Retrieve the completed pressure measurement:
					// Note that the measurement is stored in the variable P.
					// Note also that the function requires the previous temperature measurement (T).
					// (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
					// Function returns 1 if successful, 0 if failure.
					status = sensor -> getPressure (px, temp);
					if (status != 0) {
						// Print out the measurement:
						DPRINT (F("Absolute pressure: "));
						DPRINT (px, 2);
						DPRINTLN ((" millibar"));
						//Serial.print (P*0.0295333727,2);
						//Serial.println(" inHg");

						// The pressure sensor returns abolute pressure, which varies with altitude.
						// To remove the effects of altitude, use the sealevel function and your current altitude.
						// This number is commonly used in weather reports.
						// Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
						// Result: p0 = sea-level compensated pressure in mb
						//~ p0 = sensor -> sealevel (px, ALTITUDE);
						//~ DPRINT (F("Relative (sea-level) pressure: "));
						//~ DPRINT (p0, 2);
						//~ DPRINTLN (F(" millibar"));
						//Serial.print(p0*0.0295333727,2);
						//Serial.println(" inHg");

						// On the other hand, if you want to determine your altitude from the pressure reading,
						// use the altitude function along with a baseline pressure (sea-level or other).
						// Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
						// Result: a = altitude in m.
						//a = sensor -> altitude(P, p0);
						//Serial.print("Computed altitude: ");
						//Serial.print(a, 0);
						//Serial.print(" meters");
						//Serial.print(a*3.28084,0);
						//Serial.println(" feet");

						if (!isnan (temp) && !isnan (px)) {
							buf[0] = 'T';
							buf[1] = ':';
							floatToString (temp, buf + 2);
							int l = strlen (buf);
							buf[l] = ' ';
							buf[l + 1] = 'L';
							buf[l + 2] = 'P';
							buf[l + 3] = ':';
							floatToString (px, buf + l + 4);
						} else {
							buf[0] = '\0';
						}
					} else {
						DPRINTLN (F("Error retrieving pressure measurement"));
					}
				} else {
					DPRINTLN (F("Error starting pressure measurement"));
				}
			} else {
				DPRINTLN (F("Error retrieving temperature measurement"));
			}
		} else {
			DPRINTLN (F("Error starting temperature measurement"));
		}

		return buf;
	}
};
