#include "Sensor.h"


class DhtHumiditySensor: public Sensor {
private:
  DHT *dht;

  // dewPoint function (From: http://playground.arduino.cc/Main/DHT11Lib)
  double dewPoint (double celsius, double humidity) {
#ifndef USE_FAST_DEWPOINT
    // NOAA
    // reference (1) : http://wahiduddin.net/calc/density_algorithms.htm
    // reference (2) : http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
    // (1) Saturation Vapor Pressure = ESGG(T)
    double RATIO = 373.15 / (273.15 + celsius);
    double RHS = -7.90298 * (RATIO - 1);
    RHS += 5.02808 * log10(RATIO);
    RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
    RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
    RHS += log10(1013.246);

	// factor -3 is to adjust units - Vapor Pressure SVP * humidity
    double VP = pow(10, RHS - 3) * humidity;

	// (2) DEWPOINT = F(Vapor Pressure)
    double T = log(VP/0.61078);   // temp var
    return (241.88 * T) / (17.558 - T);
#else
    // delta max = 0.6544 wrt dewPoint()
    // 6.9 x faster than dewPoint()
    // reference: http://en.wikipedia.org/wiki/Dew_point
    double a = 17.271;
    double b = 237.7;
    double temp = (a * celsius) / (b + celsius) + log(humidity*0.01);
    double Td = (b * temp) / (a - temp);
    return Td;
#endif
  }

public:
	DhtHumiditySensor () {
		dht = NULL;
	}

	bool begin (FlashString name, FlashString description, DHT& _dht) {
		if (Sensor::begin (name, description, F("20160125"))) {
			dht = &_dht;
			return true;
		} else {
			return false;
		}
	}


	char *read (char *buf, const byte size _UNUSED) {
		float h = dht -> readHumidity();
		float t = dht -> readTemperature();
		if (!isnan (h) && !isnan(t)) {
			buf[0] = 'T';
			buf[1] = ':';
			floatToString (t, buf + 2);
			int l = strlen (buf);
			buf[l] = ' ';
			buf[l + 1] = 'H';
			buf[l + 2] = ':';
			floatToString (h, buf + l + 3);
		}

		return buf;
	}
};
