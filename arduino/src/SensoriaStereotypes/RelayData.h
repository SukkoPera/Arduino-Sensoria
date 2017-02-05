#ifndef RELAYDATA_H_INCLUDED
#define RELAYDATA_H_INCLUDED

#include <SensoriaCore/Stereotype.h>
#include <SensoriaCore/utils.h>
#include <SensoriaCore/debug.h>


class RelayData: public Stereotype {
public:
	enum State {
		STATE_OFF = 0,
		STATE_ON,
		STATE_UNKNOWN
	};

	State state;

	RelayData (): Stereotype ("RS") {
	}

	virtual void clear () override {
		state = STATE_UNKNOWN;
	}

	boolean unmarshal (char *s) override {
		strupr (s);
		if (strcmp_P (s, PSTR ("ON")) == 0) {
			state = STATE_ON;
		} else if (strcmp_P (s, PSTR ("OFF")) == 0) {
			state = STATE_OFF;
		} else {
			state = STATE_UNKNOWN;
		}

		return true;
	}

	char *marshal (char *buf, unsigned int size) override {
		// Start with an empty string
		if (size >= 4) {
			buf[0] = '\0';

			switch (state) {
				case STATE_ON:
					strncat_P (buf, PSTR ("ON"), size);
					break;
				case STATE_OFF:
					strncat_P (buf, PSTR ("OFF"), size);
					break;
				case STATE_UNKNOWN:
				default:
					strncat_P (buf, PSTR ("UNK"), size);
					break;
			}
		} else {
			buf = NULL;
		}

		return buf;
	}
};

#endif
