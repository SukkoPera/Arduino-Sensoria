#ifndef WEATHERDATA_H_INCLUDED
#define WEATHERDATA_H_INCLUDED

#include <SensoriaCore/Stereotype.h>
#include <SensoriaCore/utils.h>


class WeatherData: public Stereotype {
private:
	static const int MAX_SPLIT = 7;

  static const int TEMP_DELTA = 1;
  static const int LIGHT10BIT_DELTA = 75;

public:
	float temperature;
	float humidity;
	float localPressure;
	float seaPressure;
	float altitude;
	float lightLux;
	int light10bit;

	static const int UNDEFINED = -19999;

	WeatherData(): Stereotype ("WD") {
	}

  WeatherData& operator= (WeatherData& other) {
    temperature = other.temperature;
    light10bit = other.light10bit;

    return *this;
  }

  bool operator== (Stereotype const& genericOther) override {
  //~ bool operator== (WeatherData const& other) {
    WeatherData const& other = static_cast<WeatherData const&> (genericOther);
    return abs (temperature - other.temperature) <= TEMP_DELTA &&
           abs (light10bit - other.light10bit) <= LIGHT10BIT_DELTA;
  }

	virtual void clear () override {
		temperature = UNDEFINED;
		humidity = UNDEFINED;
		localPressure = UNDEFINED;
		seaPressure = UNDEFINED;
		altitude = UNDEFINED;
		lightLux = UNDEFINED;
		light10bit = UNDEFINED;
	}

	boolean unmarshal (char *s) override {
		char *p[MAX_SPLIT];
		int n = splitString (s, p, MAX_SPLIT, ' ');

		for (int i = 0; i < n; i++) {
			char *q[2];
			int m = splitString (p[i], q, 2, ':');

			if (m == 2) {
				if (strcmp_P (q[0], PSTR ("T")) == 0) {
					temperature = atof (q[1]);
				} else if (strcmp_P (q[0], PSTR ("H")) == 0) {
					humidity = atof (q[1]);
				} else if (strcmp_P (q[0], PSTR ("LP")) == 0) {
					localPressure = atof (q[1]);
				} else if (strcmp_P (q[0], PSTR ("SP")) == 0) {
					seaPressure = atof (q[1]);
				} else if (strcmp_P (q[0], PSTR ("A")) == 0) {
					altitude = atof (q[1]);
				} else if (strcmp_P (q[0], PSTR ("LX")) == 0) {
					lightLux = atof (q[1]);
				} else if (strcmp_P (q[0], PSTR ("LU")) == 0) {
					light10bit = atoi (q[1]);                  // atol?
				} else {
					// Unsupported tag
				}
			}
		}

		return true;
	}

	char *marshal (char *buf, unsigned int size) override {
		// Start with an empty string
		if (size > 0) {   // FIXME
			buf[0] = '\0';

			// Go through each member and append
			if (temperature != UNDEFINED) {
				strncat_P (buf, PSTR ("T:"), size);
				floatToString (temperature, buf + strlen (buf));
				strncat_P (buf, PSTR (" "), size);
			}

			if (humidity != UNDEFINED) {
				strncat_P (buf, PSTR ("H:"), size);
				floatToString (humidity, buf + strlen (buf));
				strncat_P (buf, PSTR (" "), size);
			}

			if (localPressure != UNDEFINED) {
				strncat_P (buf, PSTR ("LP:"), size);
				floatToString (localPressure, buf + strlen (buf));
				strncat_P (buf, PSTR (" "), size);
			}

			if (seaPressure != UNDEFINED) {
				strncat_P (buf, PSTR ("SP:"), size);
				floatToString (seaPressure, buf + strlen (buf));
				strncat_P (buf, PSTR (" "), size);
			}

			if (altitude != UNDEFINED) {
				strncat_P (buf, PSTR ("A:"), size);
				floatToString (altitude, buf + strlen (buf));
				strncat_P (buf, PSTR (" "), size);
			}

			if (lightLux != UNDEFINED) {
				strncat_P (buf, PSTR ("LX:"), size);
				floatToString (lightLux, buf + strlen (buf));
				strncat_P (buf, PSTR (" "), size);
			}

			if (light10bit != UNDEFINED) {
				strncat_P (buf, PSTR ("LU:"), size);
				floatToString (light10bit, buf + strlen (buf));
				strncat_P (buf, PSTR (" "), size);
			}

			// Remove trailing space
			buf[strlen (buf) > 0 ? strlen (buf) - 1 : 0] = '\0';
		}

		return buf;
	}
};

#endif
