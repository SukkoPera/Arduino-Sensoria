#ifndef TIMECTRLDATA_H_INCLUDED
#define TIMECTRLDATA_H_INCLUDED

#include <PString.h>
#include <SensoriaCore/Stereotype.h>
#include <SensoriaCore/utils.h>
#include <SensoriaCore/debug.h>


class TimeControlData: public Stereotype {
private:
	static const byte OFF = 0;

public:
	static const byte NDAYS = 7;
	static const byte NHOURS = 24;

	byte data[NDAYS][NHOURS];

	TimeControlData (): Stereotype ("TC") {
	}

	TimeControlData& operator= (TimeControlData& other) {
		for (byte d = 0; d < NDAYS; ++d)
			for (byte h = 0; h < NHOURS; ++h)
				data[d][h] = other.data[d][h];
		return *this;
	}

	bool operator== (Stereotype const& genericOther) override {
		TimeControlData const& other = static_cast<TimeControlData const&> (genericOther);

		bool ret = true;
		for (byte d = 0; ret && d < NDAYS; ++d) {
			for (byte h = 0; ret && h < NHOURS; ++h) {
				if (data[d][h] != other.data[d][h])
					ret = false;
			}
		}

		return ret;
	}

	virtual void clear () override {
		for (byte d = 0; d < NDAYS; ++d)
			for (byte h = 0; h < NHOURS; ++h)
				data[d][h] = OFF;
	}

	boolean unmarshal (char *s) override {
		boolean ret = true;

		strupr (s);

		char *p[NDAYS];
		byte n = splitString (s, p, NDAYS);
		for (byte i = 0; i < n; i++) {
			char *q[2];
			if (splitString (p[i], q, 2, ':') != 2) {
				DPRINTLN (F("More than one colon in day setting"));
				ret = false;
			} else {
				const char *dabbr = q[0];
				const char *sched = q[1];

				if (strlen (dabbr) != 3 || dabbr[0] != 'P') {
					DPRINT (F("Bad day abbreviation: "));
					DPRINTLN (dabbr);
					ret = false;
				} else {
					byte d = decodeDayAbbr (dabbr + 1);
					if (d >= NDAYS) {
						DPRINT (F("Bad day abbreviation: "));
						DPRINTLN (dabbr);
						ret = false;
					} else if (strlen (sched) != NHOURS) {
						DPRINTLN (F("Not exactly 24 settings per day"));
						ret = false;
					} else {
						for (byte h = 0; h < NHOURS; h++) {
							byte v = sched[h] - '0';
							data[d][h] = v;
						}
					}
				}
			}
		}

		return ret;
	}

	char *marshal (char *buf, unsigned int size) override {
		char* ret = NULL;
		size_t p = 0;

		if (size >= NDAYS * (NHOURS + 4 + 1)) {
			PString pstr (buf, size);

			for (byte d = 0; d < NDAYS; ++d) {
				p += pstr.print ('P');
				p += pstr.print (DAY_ABBREVS[d]);
				p += pstr.print (':');
				for (byte h = 0; h < NHOURS; ++h) {
					byte v = data[d][h];
					if (/* d >= 0 && */ v <= 9)
						p += pstr.print (v);
				}

				if (d < NDAYS - 1)
					p += pstr.print (' ');
			}
		}

		if (p == NDAYS * (NHOURS + 4 + 1) - 1)
			ret = buf;

		return ret;
	}

private:
	// FIXME: Consider putting these into PROGMEM
	static const char* DAY_ABBREVS[NDAYS];		// Inited in AllStereotypes.cpp

	byte decodeDayAbbr (const char* abbr) {
		byte i;

		for (i = 0; i < NDAYS; i++) {
			if (strcmp (abbr, DAY_ABBREVS[i]) == 0) {
				break;
			}
		}

		return i;
	}
};

#endif
