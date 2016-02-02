#ifndef _COMMON_H_INCLUDED
#define _COMMON_H_INCLUDED

#include <Arduino.h>

// Isn't there a ready-to-use macro in the ESP core headers??
#ifdef ESP8266_CLOCK
  //~ #warning Compiling for ESP8266
  #define PLATFORM_ESP8266
#endif

#ifndef PLATFORM_ESP8266
  // Flash strings don't seem to work very well on ESP8266
  //~ #define ENABLE_FLASH_STRINGS
#endif


#ifdef ENABLE_FLASH_STRINGS

#include <avr/pgmspace.h>

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
