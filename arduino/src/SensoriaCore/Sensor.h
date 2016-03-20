#ifndef _SENSOR_H_INCLUDED
#define _SENSOR_H_INCLUDED

#include "Transducer.h"
#include "common.h"


class Sensor: public Transducer {
public:
  Sensor (): Transducer (Transducer::SENSOR) {
  }

  virtual void configure (const char *name _UNUSED, char *value _UNUSED) {
  }
};

#endif
