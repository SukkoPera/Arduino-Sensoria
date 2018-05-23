#ifndef VALUESETDATA_H_INCLUDED
#define VALUESETDATA_H_INCLUDED

#include <stdlib.h>		// If cstdlib is used instead, min() disappears
#include <string.h>
#include <cerrno>
#include <climits>
#include <PString.h>
#include <SensoriaCore/Stereotype.h>
#include <SensoriaCore/utils.h>
#include <SensoriaCore/debug.h>

template <int N, int MAXL>
class ValueSetDataT: public Stereotype {
private:
	byte used;

	void strUnescape (char* s) {
		static const char ESCAPE_CHAR = '\\';
		char* t = s;

		while (*s) {
			if (*s != ESCAPE_CHAR || !*(s + 1)) {
				*t = *s;
			} else {
				// We are on ESCAPE_CHAR and we have a char following, get it
				char c = *++s;

				// For the moment just append the escaped char to dest string
				*t = c;
			}

			// NEXT!
			s++, t++;
		}

		// Terminate destination string
		*t = '\0';
	}

public:
	//~ static const byte UNKNOWN = -1;
	static const byte MAX_N_DATA = N;
	char intbuf[MAXL];

	char* data[MAX_N_DATA];
	byte nData;

	ValueSetDataT (): Stereotype ("VS"), used (0), nData (0) {
		intbuf[0] = '\0';
	}

	ValueSetDataT& operator= (ValueSetDataT& other) {
		for (byte i = 0; i < min (other.nData, MAX_N_DATA); ++i)
			data[i] = other.data[i];
		nData = other.nData;
		return *this;
	}

	bool operator== (Stereotype const& genericOther) override {
		ValueSetDataT const& other = static_cast<ValueSetDataT const&> (genericOther);

		bool ret = nData == other.nData;
		for (byte i = 0; ret && i < min (nData, MAX_N_DATA); ++i) {
			if (data[i] != other.data[i]) {
				ret = false;
			}
		}

		return ret;
	}

	virtual void clear () override {
		for (byte i = 0; i < min (nData, MAX_N_DATA); ++i)
			data[i] = NULL;
		nData = 0;

		intbuf[0] = '\0';
		used = 0;
	}

	boolean unmarshal (char *s) override {
		boolean ret = true;

		char* p[MAX_N_DATA];
		byte skips = 0;
		if ((nData = splitString (s, p, MAX_N_DATA, ' ')) > 0) {
			for (byte i = 0; i < nData; i++) {
				char* q[2];
				if (splitString(p[i], q, 2, ':') == 2 && toupper (q[0][0] == 'V') && strlen (q[0]) > 1) {
					int n = atoi (q[0] + 1);
					strUnescape (q[1]);     // Done in place
					data[n - skips] = q[1];
				} else {
					DPRINT (F("Bad key: "));
					DPRINTLN (p[i]);
					++skips;
				}
			}
		} else {
			// No data available
			DPRINT (F("No data available"));
			ret = false;
		}

		nData -= skips;

		return ret;
	}

	// TODO: Escape spaces!
	char *marshal (char *buf, unsigned int size) override {
		char* ret = NULL;
		PString pstr (buf, size);

		// Go through each member and append
		byte n = min (nData, MAX_N_DATA);
		boolean ok = true;
		for (byte i = 0; i < n && ok; ++i) {
			pstr.print ('V');
			pstr.print (i);
			ok = pstr.print (':') == 1;
			ok = ok && pstr.print (data[i]) == strlen (data[i]);        // TODO: Test!

			if (i < n - 1)
				pstr.print (' ');
		}

		if (ok)
			ret = buf;

		return ret;
	}

	boolean getDataInt (byte n, int& val) {
		boolean ret = false;

		char *endptr;
		errno = 0;    /* To distinguish success/failure after call */
		long lval = strtol (data[n], &endptr, 0);

		/* Check for various possible errors */
		if (errno || endptr == data[n]) {
			//~ fprintf(stderr, "No digits were found\n");
			//~ exit(EXIT_FAILURE);
		} else if (lval > INT_MAX || lval < INT_MIN) {
			// Value cannot be represented as int
		} else {
			// strtol() successfully parsed a number
			val = static_cast<int> (lval);
			ret = true;
		}

		return ret;
	}

	bool append (const char *str) {
		bool ret = true;

		size_t l = strlen (str);
		size_t avail = MAXL - used - 1;
		if (nData < MAX_N_DATA && l < avail) {
			data[nData] = intbuf + used;
			strncpy (data[nData], str, avail);
			used += l + 1;

			++nData;
		} else {
			ret = false;
		}

		return ret;
	}

	bool append (int i) {
		char buf[16];
		//~ snprintf (buf, 16, "%d", i);
		itoa (i, buf, 10);
		return append (buf);
	}
};

typedef ValueSetDataT<5, 32> ValueSetData;

#endif
