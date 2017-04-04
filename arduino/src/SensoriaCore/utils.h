#ifndef _UTILS_H_INCLUDED
#define _UTILS_H_INCLUDED

#include "common.h"

/* Remove trailing whitespace from a string.
 * String is modified in-place.
 * Returns pointer to stripped string.
 */
char *strstrip (char *s);

/* Convert a float to a string.
 *
 * NOTE: Precision is fixed to two digits.
 */
char *floatToString (double val, char *outstr);

// WARNING: str is updated in-place
int splitString (char *str, char **parts, size_t n, const char sep = ' ');

#ifdef ARDUINO_ARCH_ESP8266
char *strupr (char * s);
#endif

uint16_t crc16_update_str (uint16_t crc, const char* s);

#endif
