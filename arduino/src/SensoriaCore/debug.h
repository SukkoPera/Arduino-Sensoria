#define NDEBUG

#ifndef NDEBUG
  #define DSTART(...) Serial.begin (9600);
  #define DPRINT(...) Serial.print (__VA_ARGS__)
  #define DPRINTLN(...) Serial.println (__VA_ARGS__)
#else
  #define DSTART(...) do {} while (0)
  #define DPRINT(...) do {} while (0)
  #define DPRINTLN(...) do {} while (0)
#endif
