#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

char *strstrip (char *s) {
	for (char *c = s + strlen (s) - 1; c >= s && isspace (*c); --c)
		*c = '\0';
	return s;
}

#ifndef PLATFORM_ESP8266
// From: http://mirror.fsf.org/pmon2000/3.x/src/sdk/libc/string/strlcpy.c
size_t strlcpy (char *dst, const char *src, size_t siz) {
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';    /* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);  /* count does not include NUL */
}
#endif

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

	if (val < 0.0){
		strcpy (outstr, "-\0");  //print "-" sign
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

<<<<<<< HEAD
int splitString (char *str, char **parts, size_t n, const char sep) {
=======
int splitString (char *str, char **parts, size_t n) {
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
	size_t i;
	char *c;

	for (i = 0; i < n - 1; i++) {
		parts[i] = str;

<<<<<<< HEAD
		// Find next separator
		if ((c = strchr (str, sep))) {
			*c = '\0';	// Terminate

			// Find next non-separator
			while (*(str = ++c) == sep)
				;
		} else {
			// No more separators
=======
		// Find next space
		if ((c = strchr (str, ' '))) {
			*c = '\0';	// Terminate

			// Find next non-space
			while (*(str = ++c) == ' ')
				;
		} else {
			// No more spaces
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
			break;
		}
	}

	// Last part
	if (str)
		parts[i] = str;

	return i + 1;
}

#ifdef PLATFORM_ESP8266

char *strupr(char *s) {
  char *t = s;

  if (!s) {
    return 0;
  }

  int i = 0;
  while (*t != '\0') {
    if (*t >= 'a' && *t <= 'z') {
      *t = *t - ('a' - 'A');
    }
    t++;
  }

  return s;
}

#endif  // PLATFORM_ESP8266