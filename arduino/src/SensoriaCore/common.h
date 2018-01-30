#ifndef _COMMON_H_INCLUDED
#define _COMMON_H_INCLUDED

// Size of marshaling buffer
#define SENSOR_BUF_SIZE 32

// Uses ~1600b flash, ~120b RAM
#define ENABLE_NOTIFICATIONS

#ifndef ARDUINO_ARCH_ESP8266
  // Flash strings don't seem to work very well on ESP8266
  #define ENABLE_FLASH_STRINGS
#endif


#ifdef ENABLE_FLASH_STRINGS

#ifdef ARDUINO_ARCH_AVR
#include <avr/pgmspace.h>
#endif

#define PSTR_TO_F(s) reinterpret_cast<const __FlashStringHelper *> (s)
#define F_TO_PSTR(s) reinterpret_cast<PGM_P> (s)
#define FlashString const __FlashStringHelper *

#else

//~ #warning Flash strings disabled

#undef PSTR
#define PSTR(s) s

#undef F
#define F(s) s

#define PSTR_TO_F(s) s
#define F_TO_PSTR(s) s

#define FlashString const char *

#undef strlen_P
#define strlen_P strlen
#undef strcmp_P
#define strcmp_P strcmp
#undef strcat_P
#define strcat_P strcat

#endif

// Use to mark unused function parameters
#define _UNUSED __attribute__ ((unused))

#endif
