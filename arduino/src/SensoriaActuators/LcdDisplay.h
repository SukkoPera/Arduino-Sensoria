#include <Wire.h>
#include <LiquidCrystal.h>
#include "Actuator.h"

class LcdDisplay: public Actuator {
private:
	LCD *lcd;

	void clear () {
		lcd -> clear ();
		lcd -> home ();
	}

	void setpos (char *args) {
		char *rc[3];
		splitString (args, rc, 3);
		int row = atoi (rc[0]);
		int col = atoi (rc[1]);
		//~ Serial.print (F("Setting cursor to row "));
		//~ Serial.print (row);
		//~ Serial.print (F(" column "));
		//~ Serial.println (col);
		lcd -> setCursor (col, row);
	}

	void print (char *args) {
		lcd -> print (args);
	}

	void printAt (char *args) {
		char *rc[3];
		splitString (args, rc, 3);
		int row = atoi (rc[0]);
		int col = atoi (rc[1]);
		lcd -> setCursor (col, row);
		lcd -> print (rc[2]);
	}


public:
	LcdDisplay () {
		lcd = NULL;
	}

	virtual bool begin (const __FlashStringHelper *name, const __FlashStringHelper *description, LCD* _lcd) {
		if (_lcd != NULL && Actuator::begin (name, description)) {
			lcd = _lcd;
			return true;
		} else {
			return false;
		}
	}

	bool write (char *buf) {
		char *parts[2];
		splitString (buf, parts, 2);
		strupr (parts[0]);
		Serial.println (parts[0]);
		Serial.println (parts[1]);
		if (strcmp_P (parts[0], PSTR("CLR")) == 0) {
			clear ();
		} else if (strcmp_P (parts[0], PSTR("PRT")) == 0) {
			print (parts[1]);
		} else if (strcmp_P (parts[0], PSTR("PRP")) == 0) {
			printAt (parts[1]);
		} else if (strcmp_P (parts[0], PSTR("POS")) == 0) {
			setpos (parts[1]);
		} else {
			return false;
		}

		return true;
	}
};
