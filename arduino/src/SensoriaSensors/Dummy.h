#include "Sensor.h"

class DummySensor: public Sensor {
public:
  DummySensor (): Sensor (F("20151031"), F("DS"), F("Dummy Sensor")) {
  //~ DummySensor (): Sensor (Sensor::READ, "DS", "Dummy Sensor", "20151031") {
  }
  
  char *read (char *buf, const byte size _UNUSED) {
    buf[0] = '4';
    buf[1] = '2';
    buf[2] = '\0';
    
    return buf;
  }
};
