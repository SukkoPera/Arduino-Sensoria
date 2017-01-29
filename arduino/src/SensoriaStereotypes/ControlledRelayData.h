#ifndef CONTROLLEDRELAYDATA_H_INCLUDED
#define CONTROLLEDRELAYDATA_H_INCLUDED

#include <SensoriaCore/Stereotype.h>
#include <SensoriaCore/utils.h>
#include <SensoriaCore/debug.h>


class ControlledRelayData: public Stereotype {
public:
	enum State {
		STATE_OFF = 0,
		STATE_ON = 1,
		STATE_UNKNOWN = 255
	};

	enum Controller {
		CTRL_AUTO = 0,
		CTRL_MANUAL = 1,
		CTRL_UNKNOWN = 255
	};

	State state;
	Controller controller;

	ControlledRelayData (): Stereotype ("CR") {
	}

	virtual void clear () override {
		state = STATE_UNKNOWN;
		controller = CTRL_UNKNOWN;
	}

	//~ bool operator ==(ControlledRelayData const& other) {
		//~ return state == other.s &&
		       //~ controller == other.controller;
	//~ }

	boolean unmarshal (char *s) override {
		strupr (s);

		char *p[2];
		int n = splitString (s, p, 2, ' ');
		for (int i = 0; n == 2 && i < n; i++) {
			char *q[2];
			int m = splitString (p[i], q, 2, ':');
			if (m == 2) {
				if (strcmp_P (q[0], PSTR ("S")) == 0) {
					if (strcmp_P (q[1], PSTR ("ON")) == 0) {
						state = STATE_ON;
					} else if (strcmp_P (q[1], PSTR ("OFF")) == 0) {
						state = STATE_OFF;
					//~ } else {
						//~ state = STATE_UNKNOWN;
					}
				} else if (strcmp_P (q[0], PSTR ("C")) == 0) {
					if (strcmp_P (q[1], PSTR ("AUT")) == 0) {
						controller = CTRL_AUTO;
					} else if (strcmp_P (q[1], PSTR ("MAN")) == 0) {
						controller = CTRL_MANUAL;
					//~ } else {
						//~ controller = CTRL_UNKNOWN;
					}
				} else {
					// Unsupported tag, ignore
				}
			}
		}

		return true;
	}

	char *marshal (char *buf, unsigned int size) override {
		// Start with an empty string
		if (size >= 11) {
			buf[0] = '\0';

			strncat_P (buf, PSTR ("S:"), size);
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

			strncat_P (buf, PSTR (" C:"), size);
			switch (controller) {
				case CTRL_AUTO:
					strncat_P (buf, PSTR ("AUT"), size);
					break;
				case CTRL_MANUAL:
					strncat_P (buf, PSTR ("MAN"), size);
					break;
				case CTRL_UNKNOWN:
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
