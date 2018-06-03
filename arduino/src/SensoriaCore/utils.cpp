#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

char *strstrip (char *s) {
	for (char *c = s + strlen (s) - 1; c >= s && isspace (*c); --c)
		*c = '\0';
	return s;
}

/* This is a modified version of the floatToString posted by the Arduino forums
 * user "zitron" at http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1205038401.
 * This is slimmer than dstrtof() (350 vs. 1700 bytes!) and works well enough
 * for our needs, so here we go!
 */
char *floatToString (double val, char *outstr) {
	char temp[8];
	unsigned long frac;

	temp[0] = '\0';
	outstr[0] = '\0';

	if (val < 0.0) {
		outstr[0] = '-';  //print "-" sign
		outstr[1] = '\0';
		val *= -1;
	}

	val += 0.005;   // Round

	strcat (outstr, itoa ((int) val, temp, 10));  //prints the integer part without rounding
	strcat (outstr, ".\0");   // print the decimal point

	frac = (val - (int) val) * 100;

	if (frac < 10)
		strcat (outstr, "0\0");    // print padding zeros

	strcat (outstr, itoa (frac, temp, 10));  // print fraction part

	return outstr;
}

#ifdef ARDUINO_ARCH_STM32F1

// Yet, another good itoa implementation
// https://stackoverflow.com/questions/3440726/what-is-the-proper-way-of-implementing-a-good-itoa-function#12386915
// Modifed by SukkoPera to reflect the avrlibc function behavior, i.e.:
// The utoa() function returns the pointer passed as s.
char *itoa(int value, char *ssp, int radix) {
		char tmp[16];// be careful with the length of the buffer
		char *tp = tmp;
		char *sp = ssp;
		int i;
		unsigned v;

		int sign = (radix == 10 && value < 0);
		if (sign)
				v = -value;
		else
				v = (unsigned) value;

		while (v || tp == tmp)
		{
				i = v % radix;
				v /= radix;
				if (i < 10)
					*tp++ = i + '0';
				else
					*tp++ = i + 'a' - 10;
		}

		if (sign)
				*sp++ = '-';

		while (tp > tmp)
				*sp++ = *--tp;

		*sp = '\0';

		return ssp;
}

// Modified version of the above that handles unsigned int's
// By SukkoPera
char *utoa(unsigned int v, char *ssp, int radix) {
		char tmp[16];// be careful with the length of the buffer
		char *tp = tmp;
		char *sp = ssp;
		int i;

		while (v || tp == tmp) {
			i = v % radix;
			v /= radix;
			if (i < 10)
				*tp++ = i + '0';
			else
				*tp++ = i + 'a' - 10;
		}

		while (tp > tmp)
			*sp++ = *--tp;

		*sp = '\0';

		return ssp;
}

// Reworked from Arduino's Print.h
char *ultoa(unsigned long n, char *sp, int radix) {
	char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
	char *str = &buf[sizeof(buf) - 1];

	*str = '\0';

	// prevent crash if called with base == 1
	if (radix < 2) radix = 10;

	do {
		char c = n % radix;
		n /= radix;

		*--str = c < 10 ? c + '0' : c + 'A' - 10;
	} while(n);
	
	while (*str)
		*sp++ = *str++;
	*sp = '\0';

	return sp;
}
#endif	// ARDUINO_ARCH_STM32F1

static char *strchr_escaped (char *s, int c) {
	static const char ESCAPE_CHAR = '\\';

	char prev = ' ';
	while (*s && (*s != c || prev == ESCAPE_CHAR)) {
			prev = *s++;
	}

	return *s ? s : NULL;
}

int splitString (char *str, char **parts, size_t n, const char sep) {
	size_t i;
	char *c;

	for (i = 0; i < n - 1; i++) {
		parts[i] = str;

		// Find next separator
		if ((c = strchr_escaped (str, sep))) {
			*c = '\0';	// Terminate

			// Find next non-separator
			while (*(str = ++c) == sep)
				;
		} else {
			// No more separators
			break;
		}
	}

	// Last part
	if (str)
		parts[i] = str;

	return i + 1;
}

#ifndef ARDUINO_ARCH_AVR

char *strupr(char *s) {
	char *t = s;

	if (!s) {
		return 0;
	}

	while (*t != '\0') {
		if (*t >= 'a' && *t <= 'z') {
			*t = *t - ('a' - 'A');
		}
		t++;
	}

	return s;
}

uint16_t _crc_xmodem_update (uint16_t crc, uint8_t data) {
	int i;

	crc = crc ^ ((uint16_t) data << 8);
	for (i = 0; i < 8; i++) {
		if (crc & 0x8000)
			crc = (crc << 1) ^ 0x1021;
		else
			crc <<= 1;
	}

	return crc;
}

#else  // !ARDUINO_ARCH_AVR

// avrlib already has a CRC function we can use

#include <util/crc16.h>

#endif  // !ARDUINO_ARCH_AVR

uint16_t crc16_update_str (uint16_t crc, const char* s) {
	char c;
	while ((c = *s++)) {
		crc = _crc_xmodem_update (crc, c);
	}

	return crc;
}
