#ifndef _UTILS_H_INCLUDED
#define _UTILS_H_INCLUDED

#include <Arduino.h>
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

#ifdef ARDUINO_ARCH_STM32F1
char *itoa(int value, char *sp, int radix);
char *utoa(unsigned int value, char *sp, int radix);

// Please make sure that sp is large enough (i.e: at least 11 chars if longs are
// 32-bit)
char *ultoa(unsigned long n, char *sp, int radix);
#endif

// WARNING: str is updated in-place
int splitString (char *str, char **parts, size_t n, const char sep = ' ');

#ifdef ARDUINO_ARCH_ESP8266
char *strupr (char *s);
#endif

uint16_t crc16_update_str (uint16_t crc, const char* s);

#endif
