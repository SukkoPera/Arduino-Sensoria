#ifndef _SENSOR_H_INCLUDED
#define _SENSOR_H_INCLUDED

#include "Transducer.h"
#include "common.h"


class Sensor: public Transducer {
public:
  Sensor (): Transducer (Transducer::SENSOR) {
  }

  /* Override to implement the actual sensor reading and reporting.
   * A buffer that can be used to contain the result is provided, of
   * the given size, initialized to all-zeros. Feel free to use it or
   * provide your own. Just return whatever buffer you used.
   */
  virtual char *read (char *buf, const byte size) = 0;

  virtual void configure (const char *name _UNUSED, char *value _UNUSED) {
  }
};

#endif
