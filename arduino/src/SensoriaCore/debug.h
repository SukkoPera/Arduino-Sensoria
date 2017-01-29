#define NDEBUG

#ifndef NDEBUG
  #define DSTART(spd) Serial.begin (spd);
  #define DPRINT(...) Serial.print (__VA_ARGS__)
  #define DPRINTLN(...) Serial.println (__VA_ARGS__)
#else
  #define DSTART(...) do {} while (0)
  #define DPRINT(...) do {} while (0)
  #define DPRINTLN(...) do {} while (0)
#endif
