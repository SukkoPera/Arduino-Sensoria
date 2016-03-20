#ifndef _ACTUATOR_H_INCLUDED
#define _ACTUATOR_H_INCLUDED

#include "Transducer.h"
//~ #include <SensoriaCore/utils.h>


class Actuator: public Transducer {
private:

public:
	Actuator (): Transducer (Transducer::ACTUATOR) {
	}

	/* Override to implement the actual Actuator writing.
	 */
	virtual boolean write (Stereotype *st) = 0;

  /* Since not all actuators might be interested in this, provide a do-nothing
   * default.
   */
  virtual boolean read (Stereotype *st _UNUSED) override {
    return false;
  }

	virtual void configure (const char *name _UNUSED, char *value _UNUSED) {
	}
};

#endif
