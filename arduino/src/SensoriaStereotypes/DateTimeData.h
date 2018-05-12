#ifndef DATETIMEDATA_H_INCLUDED
#define DATETIMEDATA_H_INCLUDED

#include <SensoriaCore/Stereotype.h>
#include <SensoriaCore/utils.h>
#include <SensoriaCore/debug.h>
#include <TimeLib.h>


class DateTimeData: public Stereotype {
public:
	// date?
	// time?
	time_t unixtime;

	DateTimeData (): Stereotype ("DT") {
	}

	DateTimeData& operator= (DateTimeData& other) {
		unixtime = other.unixtime;

		return *this;
	}

	bool operator== (Stereotype const& genericOther) override {
		DateTimeData const& other = static_cast<DateTimeData const&> (genericOther);
		return unixtime == other.unixtime;
	}

	virtual void clear () override {
		unixtime = 0;
	}

	boolean unmarshal (char *s) override {
		boolean ret = true;
		strupr (s);

		char *p[3];
		int n = splitString (s, p, 3);
		for (int i = 0; n == 3 && i < n; i++) {
			char *q[2];
			int m = splitString (p[i], q, 2, ':');
			if (m == 2) {
				char& c0 = q[0][0];
				if (c0 == 'D') {
					// FIXME
				} else if (c0 == 'T') {
					// FIXME
				} else if (c0 == 'U') {
					unixtime = atol (q[1]);
				} else {
					// Unsupported tag, ignore
				}
			}
		}

		return ret;
	}

	char *marshal (char *buf, unsigned int size) override {
		ultoa (unixtime, buf, 10);
	}
};

#endif
